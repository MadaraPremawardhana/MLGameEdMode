using UnrealBuildTool;

public class MLGameRuntime : ModuleRules
{
	public MLGameRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			// Unreal's built-in Neural Network Engine - lets us load an ONNX model
			// (imported as a UNNEModelData asset) and run it on CPU/GPU at runtime.
			"NNE",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
		});
	}
}
