using UnrealBuildTool;
using System.Collections.Generic;

public class AGIS0_51EditorTarget : TargetRules
{
	public AGIS0_51EditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		ExtraModuleNames.Add("AGIS0_51");
	}
}
