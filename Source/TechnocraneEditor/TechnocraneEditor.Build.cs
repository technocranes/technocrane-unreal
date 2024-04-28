// Copyright (c) 2020 Technocrane s.r.o. 
//
// https://github.com/technocranes/technocrane-unreal
//
// TechnocraneEditor.Build.cs
// Sergei <Neill3d> Solokhin

using Path = System.IO.Path;

namespace UnrealBuildTool.Rules
{
	public class TechnocraneEditor : ModuleRules
	{
        public TechnocraneEditor(ReadOnlyTargetRules Target) : base(Target)
		{
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
            
            bLegacyPublicIncludePaths = false;

            PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
            PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

            // NOTE: for debug purpose
            //OptimizeCode = CodeOptimization.Never;

            PublicDependencyModuleNames.AddRange(
				new string[] {
                    "Core",
					"CoreUObject",
                    "Slate",
                    "SlateCore",
                    "Engine",
                    "EditorStyle",
                    "UnrealEd",
                    "MainFrame",
                    "RHI",
                    "LevelEditor",
				}
			);

            PrivateIncludePathModuleNames.AddRange(
            new string[] {
                "Settings",
                "AssetTools",
            });

            PrivateDependencyModuleNames.AddRange(
                new string[] {
                    "Projects",
                    "InputCore",
                    "UnrealEd",
                    "LevelEditor",
                    "CoreUObject",
                    "Engine",
                    "Slate",
                    "SlateCore",
                    "TechnocranePlugin"
                }
            );
        }
	}
}