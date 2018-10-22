// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using Path = System.IO.Path;

namespace UnrealBuildTool.Rules
{
	public class TechnocranePlugin : ModuleRules
	{
		public TechnocranePlugin(ReadOnlyTargetRules Target) : base(Target)
		{
            PublicDefinitions.Add("TECHNOCRANESDK_IMPORTS");

            PublicIncludePaths.AddRange(
				new string[] {
                    "TechnocranePlugin/Public",
					// ... add public include paths required here ...
				}
				);

			PrivateIncludePaths.AddRange(
				new string[] {
                    "TechnocranePlugin/Private",
					// ... add other private include paths required here ...
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
                    "Core", "Projects", "CoreUObject", "Engine", "InputCore", "CinematicCamera"
					// ... add other public dependencies that you statically link with here ...
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					// ... add private dependencies that you statically link with here ...
				}
				);

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					// ... add any modules that your module loads dynamically here ...
				}
				);

            /****************************************/

            // If you update this path, ensure the DLL runtime delay load path in FOptitrackNatnetModule::StartupModule stays in sync.
            string TechnocranePath = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "TechnocraneSDK"));
            PublicSystemIncludePaths.Add(Path.Combine(TechnocranePath, "include"));

            if (Target.Platform == UnrealTargetPlatform.Win32 || Target.Platform == UnrealTargetPlatform.Win64)
            {
                string TechnocraneLibBinPath = Path.Combine(TechnocranePath, "lib", Target.Platform == UnrealTargetPlatform.Win32 ? "Win32" : "Win64");
                PublicLibraryPaths.Add(TechnocraneLibBinPath);
                PublicAdditionalLibraries.Add("TechnocraneLib.lib");
                PublicDelayLoadDLLs.Add("TechnocraneLib.dll");
                RuntimeDependencies.Add(Path.Combine(TechnocraneLibBinPath, "TechnocraneLib.dll"));
            }
        }
	}
}
