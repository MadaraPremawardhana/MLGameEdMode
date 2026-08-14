// Copyright. All rights reserved.
#include "MLGameEdModeToolkit.h"
#include "MLGameEdMode.h"
#include "MLGameEdModeSettings.h"
#include "MLGenerationVolume.h"
#include "MLModelAsset.h"
#include "MLGameRuntime.h"

#include "Editor.h"
#include "EditorModeManager.h"
#include "Selection.h"
#include "ScopedTransaction.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"

#define LOCTEXT_NAMESPACE "FMLGameEdModeToolkit"

FMLGameEdModeToolkit::FMLGameEdModeToolkit()
{
}

void FMLGameEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	Settings = NewObject<UMLGameEdModeSettings>(GetTransientPackage(), NAME_None, RF_Transactional);

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bShowOptions = false;
	SettingsDetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	SettingsDetailsView->SetObject(Settings);

	ToolkitWidget =
		SNew(SBox)
		.Padding(6.f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Title", "ML Game - Procedural Generation"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(LOCTEXT("Help",
					"1) Set a default model + grid resolution below.\n"
					"2) Spawn an ML Generation Volume, size its box over the area to populate, "
					"assign a model and an asset palette on it.\n"
					"3) Generate. Select one or more existing volumes to batch Generate/Clear them here."))
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)
			[
				SettingsDetailsView.ToSharedRef()
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.Text(LOCTEXT("SpawnVolume", "Spawn ML Generation Volume Here"))
				.OnClicked(this, &FMLGameEdModeToolkit::OnSpawnVolumeHereClicked)
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.Text(LOCTEXT("GenerateSelected", "Generate Selected Volumes"))
				.OnClicked(this, &FMLGameEdModeToolkit::OnGenerateSelectedClicked)
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.Text(LOCTEXT("ClearSelected", "Clear Selected Volumes"))
				.OnClicked(this, &FMLGameEdModeToolkit::OnClearSelectedClicked)
			]

			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				.Text(this, &FMLGameEdModeToolkit::GetStatusText)
			]
		];

	FModeToolkit::Init(InitToolkitHost);
}

FName FMLGameEdModeToolkit::GetToolkitFName() const
{
	return FName("MLGameEdMode");
}

FText FMLGameEdModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("ToolkitName", "ML Game");
}

FEdMode* FMLGameEdModeToolkit::GetEditorMode() const
{
	return GLevelEditorModeTools().GetActiveMode(FMLGameEdMode::EM_MLGameEdModeId);
}

TSharedPtr<SWidget> FMLGameEdModeToolkit::GetInlineContent() const
{
	return ToolkitWidget;
}

FReply FMLGameEdModeToolkit::OnSpawnVolumeHereClicked()
{
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		return FReply::Handled();
	}

	const FScopedTransaction Transaction(LOCTEXT("SpawnVolumeTransaction", "Spawn ML Generation Volume"));

	FActorSpawnParameters SpawnParams;
	AMLGenerationVolume* NewVolume = World->SpawnActor<AMLGenerationVolume>(AMLGenerationVolume::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (NewVolume)
	{
		if (Settings)
		{
			NewVolume->ModelAsset = Settings->DefaultModelAsset;
			NewVolume->GridResolution = Settings->DefaultGridResolution;
		}
#if WITH_EDITOR
		NewVolume->SetActorLabel(TEXT("MLGenerationVolume"));
#endif

		if (GEditor)
		{
			GEditor->SelectNone(false, true);
			GEditor->SelectActor(NewVolume, true, true);
		}
	}

	return FReply::Handled();
}

FReply FMLGameEdModeToolkit::OnGenerateSelectedClicked()
{
	if (!GEditor)
	{
		return FReply::Handled();
	}

	const FScopedTransaction Transaction(LOCTEXT("GenerateTransaction", "Generate ML Volumes"));

	int32 Count = 0;
	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		if (AMLGenerationVolume* Volume = Cast<AMLGenerationVolume>(*It))
		{
			Volume->Modify();
			Volume->Generate();
			++Count;
		}
	}

	if (Count == 0)
	{
		UE_LOG(LogMLGame, Log, TEXT("ML Game EdMode: no AMLGenerationVolume actors selected - select one or more volumes first."));
	}

	return FReply::Handled();
}

FReply FMLGameEdModeToolkit::OnClearSelectedClicked()
{
	if (!GEditor)
	{
		return FReply::Handled();
	}

	const FScopedTransaction Transaction(LOCTEXT("ClearTransaction", "Clear ML Volumes"));

	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		if (AMLGenerationVolume* Volume = Cast<AMLGenerationVolume>(*It))
		{
			Volume->Modify();
			Volume->Clear();
		}
	}

	return FReply::Handled();
}

FText FMLGameEdModeToolkit::GetStatusText() const
{
	return LOCTEXT("StatusTip", "Tip: an ML Generation Volume's Generate/Clear buttons also work from its own Details panel.");
}

#undef LOCTEXT_NAMESPACE
