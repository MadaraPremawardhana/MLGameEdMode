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
