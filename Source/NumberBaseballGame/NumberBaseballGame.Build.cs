using UnrealBuildTool;

public class NumberBaseballGame : ModuleRules
{
	public NumberBaseballGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UMG",
			"Slate",
			"SlateCore",
			"EnhancedInput"
		});

		PublicIncludePaths.AddRange(new string[] { ModuleDirectory });
	}
}
