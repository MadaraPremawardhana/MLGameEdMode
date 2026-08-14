// Copyright. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "MLGameTypes.generated.h"

/**
 * One entry in an actor-placement palette used by procedural generation.
 * The ML model's output layer is expected to produce one score per palette
 * entry (e.g. a softmax over "empty / rock / tree / enemy spawn / loot").
 */
USTRUCT(BlueprintType)
struct FMLPaletteEntry
{
	GENERATED_BODY()

	/** Actor class to spawn when this entry "wins" for a grid cell. Leave empty to mean "spawn nothing". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ML Game")
	TSoftClassPtr<AActor> ActorClass;

	/** Optional uniform random scale range applied to spawned actors. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ML Game")
	FVector2D UniformScaleRange = FVector2D(1.f, 1.f);

	/** Human readable label, purely for editor bookkeeping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ML Game")
	FName Label;
};

/** Result of running a model once over a single grid cell. Exposed to Blueprint for custom generation logic. */
USTRUCT(BlueprintType)
struct FMLGenerationCellResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ML Game")
	FIntPoint GridCoord = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly, Category = "ML Game")
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "ML Game")
	int32 SelectedPaletteIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "ML Game")
	float Confidence = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "ML Game")
	TArray<float> RawScores;
};
