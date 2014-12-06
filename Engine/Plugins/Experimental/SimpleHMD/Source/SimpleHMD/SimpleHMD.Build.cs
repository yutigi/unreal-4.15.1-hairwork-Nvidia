// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class SimpleHMD : ModuleRules
	{
		public SimpleHMD(TargetInfo Target)
		{
			PrivateIncludePaths.AddRange(
				new string[] {
					"SimpleHMD/Private",
					"../../../../Source/Runtime/Renderer/Private",
					// ... add other private include paths required here ...
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"RHI",
					"RenderCore",
					"Renderer",
					"ShaderCore",
					"HeadMountedDisplay"
				}
				);
		}
	}
}
