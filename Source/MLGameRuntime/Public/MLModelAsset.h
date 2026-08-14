// Copyright. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MLModelAsset.generated.h"

class UNNEModelData;

/**
 * A thin, designer-friendly wrapper around a UNNEModelData asset (the asset
 * type Unreal's NNE plugin creates when you import an .onnx file).
 *
 * Why wrap it at all? Because a raw NNE model only knows about tensors -
 * flat arrays of floats in a fixed order. This asset lets you give the model's
 * inputs and outputs human-readable names (matching how you named the input
 * / output layers when you exported the model from PyTorch / TensorFlow /
 * scikit-learn / etc.), so the rest of the plugin (and your own Blueprints)
 * can talk to the model with FName keys ("Speed", "DistanceToPlayer", ...)
 * instead of raw float indices.
 *
 * HOW TO CREATE ONE:
 *   1. Train / export your model to ONNX (opset 9-17 recommended).
 *   2. Drag the .onnx file into the Content Browser - Unreal's NNE plugin
 *      will import it as a UNNEModelData asset automatically.
 *   3. Right click in the Content Browser -> Miscellaneous -> Data Asset ->
 *      pick "MLModelAsset", point its ModelData at the imported asset, and
 *      fill in InputFeatureNames / OutputFeatureNames in the exact order
 *      your model's input/output tensor expects them.
 */
UCLASS(BlueprintType)
class MLGAMERUNTIME_API UMLModelAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** The imported ONNX model (created automatically by the NNE plugin when you import a .onnx file). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ML Model")
	TObjectPtr<UNNEModelData> ModelData;

	/**
	 * Name of the NNE runtime to run this model on. "NNERuntimeORTCpu" (CPU, via ONNX Runtime)
	 * works out of the box once the NNERuntimeORT plugin is enabled and is a safe default for
	 * editor tooling and most gameplay-scale models. Swap to a GPU runtime for heavier models.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ML Model")
	FString RuntimeName = TEXT("NNERuntimeORTCpu");

	/**
	 * Names of the model's input features, in the exact order the model's input tensor expects.
	 * Anything driving this model (UMLBrainComponent, the Blueprint library, procedural generation)
	 * looks features up by these names.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ML Model")
	TArray<FName> InputFeatureNames;

	/** Names of the model's output features, in the exact order the model's output tensor produces them. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ML Model")
	TArray<FName> OutputFeatureNames;

	/** Optional free-text notes: what the model does, how it was trained, expected value ranges, etc. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ML Model", meta = (MultiLine = true))
	FString Notes;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("MLModelAsset"), GetFName());
	}
};
