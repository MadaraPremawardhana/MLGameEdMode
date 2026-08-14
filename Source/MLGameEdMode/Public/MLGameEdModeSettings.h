// Copyright. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "MLGameEdModeSettings.generated.h"

class UMLModelAsset;

/**
 * Transient settings object backing the ML Game editor mode's toolkit panel.
 * Not saved to disk - it just gives the toolkit's details view something to
 * edit, and lets "Spawn Generation Volume Here" pre-fill new volumes with a
 * sensible default model/resolution.
 */
UCLASS()
class UMLGameEdModeSettings : public UObject
{
	GENERATED_BODY()

public:
	/** Assigned automatically to any new AMLGenerationVolume spawned from this mode's toolkit. */
	UPROPERTY(EditAnywhere, Category = "ML Game")
	TObjectPtr<UMLModelAsset> DefaultModelAsset;

	/** Grid resolution assigned to any new AMLGenerationVolume spawned from this mode's toolkit. */
	UPROPERTY(EditAnywhere, Category = "ML Game")
	FIntPoint DefaultGridResolution = FIntPoint(8, 8);
};
