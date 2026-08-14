// Copyright. All rights reserved.
#pragma once

#include "CoreMinimal.h"

class UMLModelAsset;

namespace UE::NNE
{
	class IModelInstanceCPU;
}

/**
 * Thin C++ wrapper around Unreal's Neural Network Engine (NNE) that turns a
 * UMLModelAsset into something you can call with a flat array of floats in
 * and get a flat array of floats back out - i.e. exactly what UMLBrainComponent
 * and the Blueprint function library need.
 *
 * All NNE-specific API calls are isolated to MLModelRunner.cpp on purpose:
 * the exact NNE interface has shifted a little between engine minor versions
 * (5.3 / 5.4 / 5.5+), so if your engine version's NNE headers differ slightly,
 * this is the ONLY file you should need to touch to fix it up. Everything
 * else in the plugin talks to models exclusively through this class.
 *
 * Current implementation runs CPU inference synchronously (INNERuntimeCPU /
 * IModelInstanceCPU, RunSync). This is intentional: it keeps the plugin
 * dependency-light and works out of the box with the free "NNE Runtime ORT"
 * (CPU) plugin. For heavy models you may want to extend this to a GPU/async
 * runtime - the public interface below (LoadModel / RunInference) would stay
 * the same.
 */
class MLGAMERUNTIME_API FMLModelRunner
{
public:
	FMLModelRunner();
	~FMLModelRunner();

	/** Loads (or re-loads) the given model asset. Returns false and logs an error on failure. */
	bool LoadModel(const UMLModelAsset* InModelAsset);

	/** True once a model has been successfully loaded and is ready to run. */
	bool IsReady() const;

	/** Number of float inputs the currently loaded model expects (from the owning UMLModelAsset). */
	int32 GetExpectedInputCount() const { return NumInputFeatures; }

	/** Number of float outputs the currently loaded model produces (from the owning UMLModelAsset). */
	int32 GetExpectedOutputCount() const { return NumOutputFeatures; }

	/**
	 * Runs inference synchronously. InputValues must have exactly GetExpectedInputCount() entries,
	 * in the order defined by UMLModelAsset::InputFeatureNames. OutOutputValues will be resized to
	 * GetExpectedOutputCount() entries, in the order defined by UMLModelAsset::OutputFeatureNames.
	 * Returns false (and logs) if the model isn't loaded or the input count doesn't match.
	 */
	bool RunInference(const TArray<float>& InputValues, TArray<float>& OutOutputValues);

private:
	TSharedPtr<UE::NNE::IModelInstanceCPU> ModelInstance;
	int32 NumInputFeatures = 0;
	int32 NumOutputFeatures = 0;
};
