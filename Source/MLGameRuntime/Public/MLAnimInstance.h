// Copyright. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MLAnimInstance.generated.h"

class UMLProceduralAnimComponent;

/**
 * Optional Anim Instance base class that bridges UMLProceduralAnimComponent's
 * outputs straight into AnimGraph-readable variables, so you can wire them up
 * with plain "Get" variable nodes in the AnimGraph (drive a Modify Bone node's
 * translation with MLFootIK_L, blend two poses with MLBlendAlpha, etc.) with
 * no Blueprint scripting required beyond your own Anim Blueprint's graph.
 *
 * To use: make your Animation Blueprint's parent class UMLAnimInstance (or a
 * Blueprint child of it) instead of AnimInstance, make sure the owning
 * Pawn/Character has a UMLProceduralAnimComponent, and use the MLxxx
 * variables anywhere in the AnimGraph.
 */
UCLASS(Blueprintable)
class MLGAMERUNTIME_API UMLAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "ML Procedural Animation")
	float MLLeanAngle = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "ML Procedural Animation")
	float MLFootIK_L = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "ML Procedural Animation")
	float MLFootIK_R = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "ML Procedural Animation")
	float MLBlendAlpha = 0.f;

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UMLProceduralAnimComponent> CachedMLComponent;
};
