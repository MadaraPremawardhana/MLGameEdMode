// Copyright. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "MLBrainComponent.h"
#include "MLProceduralAnimComponent.generated.h"

/**
 * USE CASE: ML-driven procedural animation.
 *
 * Drop this on a Character/Pawn instead of a plain UMLBrainComponent and it
 * automatically fills in a standard set of motion-related input features
 * every tick (speed, acceleration, vertical speed, falling/crouching state),
 * runs the model, and exposes the results through simple getters. Pair it
 * with UMLAnimInstance (or read the getters yourself from your own Anim
 * Instance / Control Rig / Blueprint) to drive things like:
 *
 *   - Foot IK offsets that react to terrain/speed beyond what a curve can do
 *   - Procedural lean / counter-rotation while turning or strafing
 *   - Blending between locomotion poses based on a learned "gait" model
 *   - Secondary motion (cloth-like sway, ear/tail flick) driven by velocity
 *
 * Your model's OutputFeatureNames on the assigned ModelAsset should include
 * (as many as you use, in any order): "LeanAngle", "FootIK_L", "FootIK_R",
 * "BlendAlpha". Any you leave out of the model just read back as 0.
 */
UCLASS(ClassGroup = (MLGame), meta = (BlueprintSpawnableComponent))
class MLGAMERUNTIME_API UMLProceduralAnimComponent : public UMLBrainComponent
{
	GENERATED_BODY()

public:
	UMLProceduralAnimComponent();

	/** Convenience getters matching the conventional output feature names documented above. */
	UFUNCTION(BlueprintCallable, Category = "ML Procedural Animation")
	float GetLeanAngle() const { return GetOutputFeature(TEXT("LeanAngle")); }

	UFUNCTION(BlueprintCallable, Category = "ML Procedural Animation")
	float GetFootIKOffsetLeft() const { return GetOutputFeature(TEXT("FootIK_L")); }

	UFUNCTION(BlueprintCallable, Category = "ML Procedural Animation")
	float GetFootIKOffsetRight() const { return GetOutputFeature(TEXT("FootIK_R")); }

	UFUNCTION(BlueprintCallable, Category = "ML Procedural Animation")
	float GetBlendAlpha() const { return GetOutputFeature(TEXT("BlendAlpha")); }

protected:
	virtual void GatherInputFeatures() override;

private:
	FVector LastVelocity = FVector::ZeroVector;
};
