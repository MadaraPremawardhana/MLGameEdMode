// Copyright. All rights reserved.
#include "MLBrainComponent.h"
#include "MLModelAsset.h"
#include "MLGameRuntime.h"

UMLBrainComponent::UMLBrainComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bAutoActivate = true;
}

void UMLBrainComponent::BeginPlay()
{
	Super::BeginPlay();
	EnsureRunnerLoaded();
}

bool UMLBrainComponent::EnsureRunnerLoaded()
{
	if (bRunnerLoaded && Runner.IsReady())
	{
		return true;
	}

	if (!ModelAsset)
	{
		return false;
	}

	bRunnerLoaded = Runner.LoadModel(ModelAsset);
	return bRunnerLoaded;
}

void UMLBrainComponent::SetInputFeature(FName FeatureName, float Value)
{
	InputFeatures.Add(FeatureName, Value);
}

void UMLBrainComponent::SetInputFeatures(const TMap<FName, float>& Features)
{
	for (const TPair<FName, float>& Pair : Features)
	{
		InputFeatures.Add(Pair.Key, Pair.Value);
	}
}

bool UMLBrainComponent::RunInference()
{
	if (!EnsureRunnerLoaded())
	{
		UE_LOG(LogMLGame, Warning, TEXT("UMLBrainComponent::RunInference - '%s' has no valid ModelAsset loaded."), *GetOwner()->GetName());
		return false;
	}

	// Pack InputFeatures into a flat array in the order ModelAsset->InputFeatureNames expects.
	TArray<float> FlatInputs;
	FlatInputs.Reserve(ModelAsset->InputFeatureNames.Num());
	for (const FName& FeatureName : ModelAsset->InputFeatureNames)
	{
		if (const float* Value = InputFeatures.Find(FeatureName))
		{
			FlatInputs.Add(*Value);
		}
		else
		{
			UE_LOG(LogMLGame, VeryVerbose, TEXT("UMLBrainComponent::RunInference - missing input feature '%s', defaulting to 0."), *FeatureName.ToString());
			FlatInputs.Add(0.f);
		}
	}

	TArray<float> FlatOutputs;
	if (!Runner.RunInference(FlatInputs, FlatOutputs))
	{
		return false;
	}

	LastOutputs.Empty(FlatOutputs.Num());
	for (int32 Index = 0; Index < FlatOutputs.Num() && Index < ModelAsset->OutputFeatureNames.Num(); ++Index)
	{
		LastOutputs.Add(ModelAsset->OutputFeatureNames[Index], FlatOutputs[Index]);
	}

	OnInferenceComplete.Broadcast();
	return true;
}

float UMLBrainComponent::GetOutputFeature(FName FeatureName) const
{
	if (const float* Value = LastOutputs.Find(FeatureName))
	{
		return *Value;
	}
	UE_LOG(LogMLGame, VeryVerbose, TEXT("UMLBrainComponent::GetOutputFeature - '%s' not found in last outputs."), *FeatureName.ToString());
	return 0.f;
}

FName UMLBrainComponent::GetTopOutputFeature() const
{
	FName Best = NAME_None;
	float BestValue = -TNumericLimits<float>::Max();
	for (const TPair<FName, float>& Pair : LastOutputs)
	{
		if (Pair.Value > BestValue)
		{
			BestValue = Pair.Value;
			Best = Pair.Key;
		}
	}
	return Best;
}

void UMLBrainComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bAutoRunOnTick)
	{
		return;
	}

	TimeSinceLastInference += DeltaTime;
	if (TimeSinceLastInference < InferenceIntervalSeconds)
	{
		return;
	}
	TimeSinceLastInference = 0.f;

	GatherInputFeatures();
	RunInference();
}
