using UnrealBuildTool;
using System.Collections.Generic;

public class AGIS0_51Target : TargetRules
{
	public AGIS0_51Target(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		ExtraModuleNames.Add("AGIS0_51");
	}
}
