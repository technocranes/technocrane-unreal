// Copyright (c) 2019 Technocrane s.r.o. 
//
// TechnocranePlugin.Build.cs
// Sergei <Neill3d> Solokhin 2019

using UnrealBuildTool;
using System;
using System.IO;

using Path = System.IO.Path;

namespace UnrealBuildTool.Rules
{
	public class TechnocranePlugin : ModuleRules
	{
        
        public TechnocranePlugin(ReadOnlyTargetRules Target) : base(Target)
		{
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
            bEnforceIWYU = true;
            bLegacyPublicIncludePaths = false;

            if (!Target.bUseUnityBuild)
            {
                PrivatePCHHeaderFile = "Private/TechnocranePrivatePCH.h";
#if UE_4_22_OR_LATER
#else
                PrivateDependencyModuleNames.Add("LivePP");
#endif
            }

            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                PublicDefinitions.Add("TECHNOCRANESDK");
                PublicDefinitions.Add("TECHNOCRANESDK_IMPORTS");
            }

            PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
            PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

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

            // If you update this path, ensure the DLL runtime delay load path in TechnocraneModule::StartupModule stays in sync.
            string TechnocranePath = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "ThirdParty", "TechnocraneSDK"));
            PublicSystemIncludePaths.Add(Path.Combine(TechnocranePath, "include"));

            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                string TechnocraneLibBinPath = Path.Combine(TechnocranePath, "lib", "Win64");
                PublicLibraryPaths.Add(TechnocraneLibBinPath);
                PublicAdditionalLibraries.Add(Path.Combine(TechnocraneLibBinPath, "TechnocraneLib.lib"));

                PublicDelayLoadDLLs.Add("TechnocraneLib.dll");
                // Add API for Runtime too
                RuntimeDependencies.Add(Path.Combine(TechnocraneLibBinPath, "TechnocraneLib.lib"));
                RuntimeDependencies.Add(Path.Combine(TechnocraneLibBinPath, "TechnocraneLib.dll"));
            }
        }
    }
}
