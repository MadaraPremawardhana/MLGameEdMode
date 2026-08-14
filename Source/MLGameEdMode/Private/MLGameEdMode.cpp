// Copyright. All rights reserved.
#include "MLGameEdMode.h"
#include "MLGameEdModeToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "EditorModeManager.h"

const FEditorModeID FMLGameEdMode::EM_MLGameEdModeId = TEXT("EM_MLGameEdModeId");

FMLGameEdMode::FMLGameEdMode()
{
}

FMLGameEdMode::~FMLGameEdMode()
{
}

void FMLGameEdMode::Enter()
{
	FEdMode::Enter();

	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Toolkit = MakeShareable(new FMLGameEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}
}

void FMLGameEdMode::Exit()
{
	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	FEdMode::Exit();
}
