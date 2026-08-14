// Copyright. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MLGameTypes.h"
#include "MLModelRunner.h"
#include "MLGenerationVolume.generated.h"

class UBoxComponent;
class UMLModelAsset;

/**
 * USE CASE: ML-driven procedural environment generation.
 *
 * Place this actor in a level, size its box to cover the area you want to
 * populate, point it at a UMLModelAsset and fill in a palette of actor
 * classes (rocks, trees, enemy spawners, loot, ...). Generate() walks a grid
 * over the box, runs the model once per cell with that cell's normalized
 * position (and any extra features you add) as input, interprets the model's
 * output as a score per palette entry, and spawns the winning entry directly
 * into the level - this is the actor that actually connects your model to
 * the game level.
 *
 * Model contract:
 *   InputFeatureNames  -> at minimum "NormX", "NormY" (cell position in
 *                         [0,1] across the box). Add more (noise, biome id,
 *                         distance-to-poi, ...) and set them via
 *                         ExtraInputFeatures before calling Generate().
 *   OutputFeatureNames -> one entry per AssetPalette entry, in the same
 *                         order. Highest score wins for that cell (softmax
 *                         + weighted random if bUseWeightedRandomSelection).
 *
 * Works both at runtime (BeginPlay, if bGenerateOnBeginPlay) and in the
 * editor: Generate() / Clear() are CallInEditor, so they show up as buttons
 * in the Details panel, and the ML Game editor mode's toolkit can batch-run
 * them across every selected volume in the level.
 */
UCLASS(Blueprintable)
class MLGAMERUNTIME_API AMLGenerationVolume : public AActor
{
	GENERATED_BODY()

public:
	AMLGenerationVolume();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ML Generation")
	TObjectPtr<UBoxComponent> Bounds;

	/** Model whose outputs select which palette entry (if any) spawns in each grid cell. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ML Generation")
	TObjectPtr<UMLModelAsset> ModelAsset;

	/** What can be spawned. Output feature N of ModelAsset corresponds to AssetPalette[N]. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ML Generation")
	TArray<FMLPaletteEntry> AssetPalette;

	/** How many cells to sample across the box, in X and Y. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ML Generation", meta = (ClampMin = "1"))
	FIntPoint GridResolution = FIntPoint(8, 8);

	/** Extra named input features passed to the model alongside NormX/NormY for every cell (e.g. a biome id, a seed value). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ML Generation")
	TMap<FName, float> ExtraInputFeatures;

	/** If true, picks the palette entry via weighted random sampling of the softmax'd scores instead of always taking the argmax. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ML Generation")
	bool bUseWeightedRandomSelection = true;

	/** Cells whose winning score (after softmax) is below this are left empty, regardless of which entry "won". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ML Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinConfidenceToSpawn = 0.f;

	/** If true, each cell is line-traced down onto world geometry and the spawned actor is placed on the hit surface. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ML Generation")
	bool bTraceToGround = true;

	/** Random seed for jitter/scale/weighted-selection. Same seed + same model = reproducible layouts. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ML Generation")
	int32 RandomSeed = 1337;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ML Generation")
	bool bGenerateOnBeginPlay = false;

	/** Runs the model across the grid and spawns actors into the level. Clears any previous generation from this volume first. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ML Generation")
	void Generate();

	/** Destroys everything previously spawned by this volume's Generate() call. */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ML Generation")
	void Clear();

	/** Read-only results of the most recent Generate() call, useful for debug visualisation or custom spawn logic in Blueprint. */
	UFUNCTION(BlueprintCallable, Category = "ML Generation")
	const TArray<FMLGenerationCellResult>& GetLastResults() const { return LastResults; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> SpawnedActors;

	TArray<FMLGenerationCellResult> LastResults;

	FMLModelRunner Runner;
};
