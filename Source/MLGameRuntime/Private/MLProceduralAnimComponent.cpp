// Copyright. All rights reserved.
#include "MLProceduralAnimComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UMLProceduralAnimComponent::UMLProceduralAnimComponent()
{
	bAutoRunOnTick = true;
	InferenceIntervalSeconds = 0.f; // procedural animation wants to run every tick by default
}

void UMLProceduralAnimComponent::GatherInputFeatures()
{
	const APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!OwningPawn)
	{
		return;
	}

	const FVector Velocity = OwningPawn->GetVelocity();
	const float DeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.f;
	const FVector Acceleration = DeltaTime > KINDA_SMALL_NUMBER ? (Velocity - LastVelocity) / DeltaTime : FVector::ZeroVector;
	LastVelocity = Velocity;

	SetInputFeature(TEXT("Speed"), Velocity.Size2D());
	SetInputFeature(TEXT("VerticalSpeed"), Velocity.Z);
	SetInputFeature(TEXT("AccelerationX"), Acceleration.X);
	SetInputFeature(TEXT("AccelerationY"), Acceleration.Y);

	if (const ACharacter* Character = Cast<ACharacter>(OwningPawn))
	{
		if (const UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			SetInputFeature(TEXT("IsFalling"), Movement->IsFalling() ? 1.f : 0.f);
			SetInputFeature(TEXT("IsCrouching"), Movement->IsCrouching() ? 1.f : 0.f);
		}
	}

	const float YawRad = FMath::DegreesToRadians(OwningPawn->GetControlRotation().Yaw - OwningPawn->GetActorRotation().Yaw);
	SetInputFeature(TEXT("AimOffsetYaw"), FMath::Sin(YawRad));
}
