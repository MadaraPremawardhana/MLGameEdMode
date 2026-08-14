// Copyright. All rights reserved.
#include "MLGameBlueprintLibrary.h"
#include "MLModelAsset.h"
#include "MLModelRunner.h"
#include "MLGameRuntime.h"

bool UMLGameBlueprintLibrary::RunModelOnce(UMLModelAsset* ModelAsset, const TMap<FName, float>& Inputs, TMap<FName, float>& Outputs)
{
	Outputs.Empty();

	if (!ModelAsset)
	{
		UE_LOG(LogMLGame, Warning, TEXT("RunModelOnce - no ModelAsset given."));
		return false;
	}

	FMLModelRunner Runner;
	if (!Runner.LoadModel(ModelAsset))
	{
		return false;
	}

	TArray<float> FlatInputs;
	FlatInputs.Reserve(ModelAsset->InputFeatureNames.Num());
	for (const FName& FeatureName : ModelAsset->InputFeatureNames)
	{
		if (const float* Value = Inputs.Find(FeatureName))
		{
			FlatInputs.Add(*Value);
		}
		else
		{
			UE_LOG(LogMLGame, Warning, TEXT("RunModelOnce - missing input feature '%s', defaulting to 0."), *FeatureName.ToString());
			FlatInputs.Add(0.f);
		}
	}

	TArray<float> FlatOutputs;
	if (!Runner.RunInference(FlatInputs, FlatOutputs))
	{
		return false;
	}

	for (int32 Index = 0; Index < FlatOutputs.Num() && Index < ModelAsset->OutputFeatureNames.Num(); ++Index)
	{
		Outputs.Add(ModelAsset->OutputFeatureNames[Index], FlatOutputs[Index]);
	}

	return true;
}

FName UMLGameBlueprintLibrary::GetHighestScoringKey(const TMap<FName, float>& Scores, float& OutScore)
{
	FName Best = NAME_None;
	OutScore = -TNumericLimits<float>::Max();
	for (const TPair<FName, float>& Pair : Scores)
	{
		if (Pair.Value > OutScore)
		{
			OutScore = Pair.Value;
			Best = Pair.Key;
		}
	}
	return Best;
}

TArray<float> UMLGameBlueprintLibrary::SoftmaxNormalize(const TArray<float>& RawScores)
{
	TArray<float> Result;
	Result.SetNumZeroed(RawScores.Num());
	if (RawScores.Num() == 0)
	{
		return Result;
	}

	float MaxScore = -TNumericLimits<float>::Max();
	for (float Score : RawScores)
	{
		MaxScore = FMath::Max(MaxScore, Score);
	}

	float Sum = 0.f;
	for (int32 Index = 0; Index < RawScores.Num(); ++Index)
	{
		const float Exp = FMath::Exp(RawScores[Index] - MaxScore);
		Result[Index] = Exp;
		Sum += Exp;
	}

	if (Sum > KINDA_SMALL_NUMBER)
	{
		for (float& Value : Result)
		{
			Value /= Sum;
		}
	}

	return Result;
}
