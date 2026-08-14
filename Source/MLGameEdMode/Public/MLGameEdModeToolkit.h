// Copyright. All rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Toolkits/BaseToolkit.h"

class IDetailsView;
class UMLGameEdModeSettings;
class SWidget;

/**
 * Slate panel shown while the ML Game editor mode is active. Lets you set a
 * default model asset / grid resolution, spawn new AMLGenerationVolume
 * actors into the level, and batch Generate()/Clear() every selected
 * AMLGenerationVolume at once (handy once a level has more than one).
 */
class FMLGameEdModeToolkit : public FModeToolkit
{
public:
	FMLGameEdModeToolkit();

	// FModeToolkit / IToolkit interface
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost) override;
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual class FEdMode* GetEditorMode() const override;
	virtual TSharedPtr<SWidget> GetInlineContent() const override;

private:
	TSharedPtr<SWidget> ToolkitWidget;
	TSharedPtr<IDetailsView> SettingsDetailsView;
	TObjectPtr<UMLGameEdModeSettings> Settings;

	FReply OnGenerateSelectedClicked();
	FReply OnClearSelectedClicked();
	FReply OnSpawnVolumeHereClicked();

	FText GetStatusText() const;
};
