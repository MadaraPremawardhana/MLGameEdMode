// Copyright. All rights reserved.
#include "MLGameEdModeModule.h"
#include "MLGameEdMode.h"
#include "EditorModeRegistry.h"
#include "Styling/SlateIcon.h"

#define LOCTEXT_NAMESPACE "FMLGameEdModeModule"

void FMLGameEdModeModule::StartupModule()
{
	FEditorModeRegistry::Get().RegisterMode<FMLGameEdMode>(
		FMLGameEdMode::EM_MLGameEdModeId,
		LOCTEXT("MLGameEdModeName", "ML Game"),
		FSlateIcon(),
		/*bVisible=*/ true);
}

void FMLGameEdModeModule::ShutdownModule()
{
	FEditorModeRegistry::Get().UnregisterMode(FMLGameEdMode::EM_MLGameEdModeId);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMLGameEdModeModule, MLGameEdMode)
