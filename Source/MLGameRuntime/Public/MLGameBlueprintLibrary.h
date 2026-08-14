// Copyright. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MLGameBlueprintLibrary.generated.h"

class UMLModelAsset;

/**
 * Stateless helpers for calling an ML model from anywhere in Blueprint or C++
 * (GameMode, GameInstance, a widget, an AnimNotify, wherever) without needing
 * to add a UMLBrainComponent. Good for one-off / infrequent calls; for
 * anything running every tick on an actor, prefer UMLBrainComponent instead
 * since it caches the loaded model.
 */
UCLASS()
class MLGAMERUNTIME_API UMLGameBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Loads ModelAsset (if not already cached) and runs it once against Inputs.
	 * Inputs/Outputs are keyed by the feature names configured on the ModelAsset.
	 * Returns false on failure (see Output Log, category LogMLGame).
	 *
	 * NOTE: this reloads the model from ModelAsset each call is expensive-ish
	 * (NNE model instance creation), consider a UMLBrainComponent for anything
	 * called every frame.
	 */
	UFUNCTION(BlueprintCallable, Category = "ML Game", meta = (DisplayName = "Run ML Model Once"))
	static bool RunModelOnce(UMLModelAsset* ModelAsset, const TMap<FName, float>& Inputs, TMap<FName, float>& Outputs);

	/** Returns the FName of the highest-value entry in a float map, e.g. picking the winning class from a classifier's outputs. */
	UFUNCTION(BlueprintCallable, Category = "ML Game")
	static FName GetHighestScoringKey(const TMap<FName, float>& Scores, float& OutScore);

	/** Simple 1D softmax-style normalisation of raw model scores into probabilities that sum to 1. Useful before weighted-random sampling. */
	UFUNCTION(BlueprintCallable, Category = "ML Game")
	static TArray<float> SoftmaxNormalize(const TArray<float>& RawScores);
};
