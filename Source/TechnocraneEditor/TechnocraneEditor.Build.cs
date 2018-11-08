// Copyright (c) 2018 Technocrane s.r.o. 
//
// TechnocraneEditor.Build.cs
// Sergei <Neill3d> Solokhin 2018

namespace UnrealBuildTool.Rules
{
	public class TechnocraneEditor : ModuleRules
	{
        public TechnocraneEditor(ReadOnlyTargetRules Target) : base(Target)
		{
            OptimizeCode = CodeOptimization.Never;

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
                    "TechnocranePlugin"
				}
			);

            PrivateIncludePathModuleNames.AddRange(
            new string[] {
                "AssetTools",
            });

            PrivateDependencyModuleNames.AddRange(
                new string[] {
                    "TechnocranePlugin"
                }
            );
        }
	}
}