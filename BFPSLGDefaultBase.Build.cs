// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class BFPSLGDefaultBase : ModuleRules
{
	public BFPSLGDefaultBase(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] 
        {
            "Core",
            "CoreUObject",
            "Engine",
            "Slate",
            "AssetTools",
            "UnrealEd",
        });

		PrivateDependencyModuleNames.AddRange(new string[] 
        {
            "Projects",
            "InputCore",
            "SlateCore",
            "LevelEditor",
            "PropertyEditor",
            "KismetWidgets",
            "KismetCompiler",
            "BlueprintGraph",
            "GraphEditor",
            "Kismet",
            "EditorStyle",
            "Slate",
            "Sequencer",
            "DetailCustomizations",
            "Settings",
            "RenderCore",
        });
	}
}
