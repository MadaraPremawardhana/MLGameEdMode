// Copyright. All rights reserved.
#include "MLAnimInstance.h"
#include "MLProceduralAnimComponent.h"
#include "GameFramework/Pawn.h"

void UMLAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (const APawn* OwningPawn = Cast<APawn>(TryGetPawnOwner()))
	{
		CachedMLComponent = OwningPawn->FindComponentByClass<UMLProceduralAnimComponent>();
	}
}

void UMLAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!CachedMLComponent)
	{
		if (const APawn* OwningPawn = Cast<APawn>(TryGetPawnOwner()))
		{
			CachedMLComponent = OwningPawn->FindComponentByClass<UMLProceduralAnimComponent>();
		}
	}

	if (!CachedMLComponent)
	{
		return;
	}

	MLLeanAngle = CachedMLComponent->GetLeanAngle();
	MLFootIK_L = CachedMLComponent->GetFootIKOffsetLeft();
	MLFootIK_R = CachedMLComponent->GetFootIKOffsetRight();
	MLBlendAlpha = CachedMLComponent->GetBlendAlpha();
}
