using UnrealBuildTool;

public class AGIS0_51 : ModuleRules
{
	public AGIS0_51(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"AIModule",
			"NavigationSystem",
			"GameplayTasks",
			"EnhancedInput",
			"UMG",
			"Slate",
			"SlateCore"
		});
	}
}
