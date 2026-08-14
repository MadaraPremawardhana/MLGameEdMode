// Copyright. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"

/**
 * The "ML Game" editor mode. Adds an icon to the level editor's mode toolbar
 * (Modeling Mode, Landscape Mode, etc. live in the same toolbar) that opens
 * a panel for batch-driving ML-based procedural generation across the level
 * - see FMLGameEdModeToolkit for the actual UI.
 *
 * Deliberately built on the classic FEdMode API (still fully supported in
 * UE 5.3+, module UnrealEd, header "EdMode.h") rather than the newer
 * UEdMode / Interactive Tools Framework path, since it's the more stable,
 * better-documented surface for a mode whose job is "show a panel with some
 * buttons" rather than "add new interactive viewport gizmos/tools".
 */
class MLGAMEEDMODE_API FMLGameEdMode : public FEdMode
{
public:
	/** Unique ID this mode is registered under - see FMLGameEdModeModule::StartupModule. */
	const static FEditorModeID EM_MLGameEdModeId;

	FMLGameEdMode();
	virtual ~FMLGameEdMode();

	// FEdMode interface
	virtual void Enter() override;
	virtual void Exit() override;
	virtual bool UsesToolkits() const override { return true; }
	virtual bool ShouldDrawWidget() const override { return false; }
};
