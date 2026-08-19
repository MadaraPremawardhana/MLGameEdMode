// Copyright. All rights reserved.
#include "MLModelRunner.h"
#include "MLModelAsset.h"
#include "MLGameRuntime.h"

#include "NNE.h"
#include "NNERuntimeCPU.h"
#include "NNEModelData.h"

FMLModelRunner::FMLModelRunner() = default;
FMLModelRunner::~FMLModelRunner() = default;

bool FMLModelRunner::LoadModel(const UMLModelAsset* InModelAsset)
{
	ModelInstance.Reset();
	NumInputFeatures = 0;
	NumOutputFeatures = 0;

	if (!InModelAsset)
	{
		UE_LOG(LogMLGame, Error, TEXT("FMLModelRunner::LoadModel - no UMLModelAsset given."));
		return false;
	}

	if (!InModelAsset->ModelData)
	{
		UE_LOG(LogMLGame, Error, TEXT("FMLModelRunner::LoadModel - '%s' has no ModelData (import a .onnx file and assign it)."), *InModelAsset->GetName());
		return false;
	}

	if (InModelAsset->InputFeatureNames.Num() == 0 || InModelAsset->OutputFeatureNames.Num() == 0)
	{
		UE_LOG(LogMLGame, Error, TEXT("FMLModelRunner::LoadModel - '%s' has no Input/OutputFeatureNames configured."), *InModelAsset->GetName());
		return false;
	}

	const FString RuntimeName = InModelAsset->RuntimeName.IsEmpty() ? TEXT("NNERuntimeORTCpu") : InModelAsset->RuntimeName;

	TWeakInterfacePtr<INNERuntimeCPU> Runtime = UE::NNE::GetRuntime<INNERuntimeCPU>(RuntimeName);
	if (!Runtime.IsValid())
	{
		UE_LOG(LogMLGame, Error, TEXT("FMLModelRunner::LoadModel - NNE runtime '%s' not found. Enable the 'NNE Runtime ORT' (and/or 'NNE Runtime Basic Cpu') plugin under Edit > Plugins and restart the editor."), *RuntimeName);
		return false;
	}

	// UE 5.3 NNE API: INNERuntimeCPU::CreateModel() -> TUniquePtr<IModelCPU>,
	// IModelCPU::CreateModelInstance() -> TUniquePtr<IModelInstanceCPU>.
	// (UE 5.4+ renamed these to CreateModelCPU()/CreateModelInstanceCPU() and
	// switched to TSharedPtr - swap the two calls below if you upgrade.)
	TUniquePtr<UE::NNE::IModelCPU> Model = Runtime->CreateModel(InModelAsset->ModelData);
	if (!Model.IsValid())
	{
		UE_LOG(LogMLGame, Error, TEXT("FMLModelRunner::LoadModel - runtime '%s' failed to create a model from '%s'. Check the .onnx file imported cleanly."), *RuntimeName, *InModelAsset->GetName());
		return false;
	}

	ModelInstance = Model->CreateModelInstance();
	if (!ModelInstance.IsValid())
	{
		UE_LOG(LogMLGame, Error, TEXT("FMLModelRunner::LoadModel - failed to create a model instance for '%s'."), *InModelAsset->GetName());
		return false;
	}

	NumInputFeatures = InModelAsset->InputFeatureNames.Num();
	NumOutputFeatures = InModelAsset->OutputFeatureNames.Num();

	// Assume a single flat input tensor of shape [1, NumInputFeatures] (batch size 1).
	// If your model uses a different input layout, this is the line to change.
	TArray<uint32> InputShapeDims = { 1u, (uint32)NumInputFeatures };
	TArray<UE::NNE::FTensorShape> InputShapes;
	InputShapes.Add(UE::NNE::FTensorShape::Make(InputShapeDims));

	if (ModelInstance->SetInputTensorShapes(InputShapes) != 0)
	{
		UE_LOG(LogMLGame, Error, TEXT("FMLModelRunner::LoadModel - SetInputTensorShapes failed for '%s'. Does the model expect a [1, %d] input tensor?"), *InModelAsset->GetName(), NumInputFeatures);
		ModelInstance.Reset();
		return false;
	}

	UE_LOG(LogMLGame, Log, TEXT("FMLModelRunner::LoadModel - loaded '%s' on runtime '%s' (%d inputs -> %d outputs)."), *InModelAsset->GetName(), *RuntimeName, NumInputFeatures, NumOutputFeatures);
	return true;
}

bool FMLModelRunner::IsReady() const
{
	return ModelInstance.IsValid();
}

bool FMLModelRunner::RunInference(const TArray<float>& InputValues, TArray<float>& OutOutputValues)
{
	if (!IsReady())
	{
		UE_LOG(LogMLGame, Warning, TEXT("FMLModelRunner::RunInference - called with no model loaded."));
		return false;
	}

	if (InputValues.Num() != NumInputFeatures)
	{
		UE_LOG(LogMLGame, Warning, TEXT("FMLModelRunner::RunInference - expected %d input values, got %d."), NumInputFeatures, InputValues.Num());
		return false;
	}

	OutOutputValues.SetNumZeroed(NumOutputFeatures);

	UE::NNE::FTensorBindingCPU InputBinding;
	InputBinding.Data = (void*)InputValues.GetData();
	InputBinding.SizeInBytes = InputValues.Num() * sizeof(float);

	UE::NNE::FTensorBindingCPU OutputBinding;
	OutputBinding.Data = (void*)OutOutputValues.GetData();
	OutputBinding.SizeInBytes = OutOutputValues.Num() * sizeof(float);

	TArray<UE::NNE::FTensorBindingCPU> Inputs{ InputBinding };
	TArray<UE::NNE::FTensorBindingCPU> Outputs{ OutputBinding };

	const int32 Result = ModelInstance->RunSync(Inputs, Outputs);
	if (Result != 0)
	{
		UE_LOG(LogMLGame, Warning, TEXT("FMLModelRunner::RunInference - RunSync returned error code %d."), Result);
		return false;
	}

	return true;
}
