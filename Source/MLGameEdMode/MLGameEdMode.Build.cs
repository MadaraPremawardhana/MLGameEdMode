using UnrealBuildTool;

public class MLGameEdMode : ModuleRules
{
	public MLGameEdMode(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"MLGameRuntime",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"UnrealEd",
			// FEditorModeInfo (used by FEditorModeRegistry::RegisterMode) and FToolkitManager
			// live in this module in UE5, separate from UnrealEd - without it you get LNK2019
			// "unresolved external symbol" errors for FEditorModeInfo::FEditorModeInfo,
			// FToolkitManager::Get and FToolkitManager::CloseToolkit at link time.
			"EditorFramework",
			"EditorStyle",
			"EditorSubsystem",
			"InputCore",
			"PropertyEditor",
			"ToolMenus",
			"Projects",
			"NNE",
		});
	}
}
