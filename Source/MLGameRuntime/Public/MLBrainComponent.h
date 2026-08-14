// Copyright. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MLModelRunner.h"
#include "MLBrainComponent.generated.h"

class UMLModelAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMLInferenceComplete);

/**
 * Generic, reusable "ML brain" you can add to any actor: point it at a
 * UMLModelAsset, feed it named input features from Blueprint or C++, and
 * read named output features back out. This is the core building block the
 * rest of the plugin (procedural animation, procedural generation) is built
 * on top of, and it's also the one you'll want for use cases the plugin
 * doesn't ship a bespoke class for, e.g.:
 *
 *   - NPC behaviour / decision making: feed in distance-to-player, health,
 *     ammo, line-of-sight bool, etc.; read out action scores (attack / flee /
 *     take cover / patrol) and act on whichever scores highest.
 *   - Dynamic difficulty adjustment: feed in recent deaths, time-to-kill,
 *     accuracy; read out a difficulty multiplier and apply it to enemy stats
 *     or spawn rates.
 *   - Anomaly / cheat detection: feed in a rolling window of player speed,
 *     acceleration and input timing; read out an "anomaly score" and flag
 *     or log players who cross a threshold.
 *   - Player behaviour prediction / churn: feed in session stats; read out a
 *     predicted engagement score to drive analytics or adaptive content.
 *
 * Usage:
 *   1. Add a UMLBrainComponent (or a Blueprint subclass of it) to an actor.
 *   2. Assign ModelAsset.
 *   3. Each tick (or manually) call SetInputFeature(Name, Value) for each of
 *      the model's InputFeatureNames, then call RunInference().
 *   4. Read results with GetOutputFeature(Name), or bind to OnInferenceComplete.
 */
UCLASS(ClassGroup = (MLGame), meta = (BlueprintSpawnableComponent))
class MLGAMERUNTIME_API UMLBrainComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMLBrainComponent();

	/** The model this brain runs. Must have InputFeatureNames / OutputFeatureNames configured. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ML Brain")
	TObjectPtr<UMLModelAsset> ModelAsset;

	/** If true, automatically gathers inputs (via GatherInputFeatures, overridable in subclasses) and runs inference every tick. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ML Brain")
	bool bAutoRunOnTick = false;

	/** Only relevant when bAutoRunOnTick is true. 0 = every tick. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ML Brain", meta = (ClampMin = "0.0"))
	float InferenceIntervalSeconds = 0.1f;

	/** Broadcast every time RunInference() completes successfully. Read outputs with GetOutputFeature(). */
	UPROPERTY(BlueprintAssignable, Category = "ML Brain")
	FOnMLInferenceComplete OnInferenceComplete;

	/** Sets (or overwrites) one named input feature. Names must match an entry in ModelAsset->InputFeatureNames. */
	UFUNCTION(BlueprintCallable, Category = "ML Brain")
	void SetInputFeature(FName FeatureName, float Value);

	/** Bulk version of SetInputFeature. */
	UFUNCTION(BlueprintCallable, Category = "ML Brain")
	void SetInputFeatures(const TMap<FName, float>& Features);

	/** Runs the model synchronously against the currently set input features. Returns false on failure (see log). */
	UFUNCTION(BlueprintCallable, Category = "ML Brain")
	bool RunInference();

	/** Reads back a named output feature from the last successful inference. Returns 0 and logs a warning if not found. */
	UFUNCTION(BlueprintCallable, Category = "ML Brain")
	float GetOutputFeature(FName FeatureName) const;

	/** All output features from the last successful inference. */
	UFUNCTION(BlueprintCallable, Category = "ML Brain")
	const TMap<FName, float>& GetAllOutputFeatures() const { return LastOutputs; }

	/** Index of the output feature with the highest value (useful for classification-style models). INDEX_NONE if no outputs yet. */
	UFUNCTION(BlueprintCallable, Category = "ML Brain")
	FName GetTopOutputFeature() const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	/**
	 * Override in a subclass to automatically populate InputFeatures from the owning actor's state
	 * before each auto-run tick (see UMLProceduralAnimComponent for an example). Base implementation
	 * does nothing - by default you're expected to call SetInputFeature(s) yourself.
	 */
	virtual void GatherInputFeatures() {}

private:
	FMLModelRunner Runner;
	bool bRunnerLoaded = false;

	TMap<FName, float> InputFeatures;
	TMap<FName, float> LastOutputs;

	float TimeSinceLastInference = 0.f;

	bool EnsureRunnerLoaded();
};
