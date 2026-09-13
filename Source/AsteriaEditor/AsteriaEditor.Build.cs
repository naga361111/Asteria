// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class AsteriaEditor : ModuleRules
{
	public AsteriaEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// UE5.8 no longer adds the module root to include paths automatically,
		// so subfolder includes like "BuildMode/AsteriaBuildEdMode.h" (relative to
		// the module root) fail to resolve. Add the module root back explicitly.
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[]
			{ "Core", "CoreUObject", "Engine" });

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate", "SlateCore", "InputCore",
			"UnrealEd", "EditorFramework", "LevelEditor",
			"InteractiveToolsFramework", "EditorInteractiveToolsFramework"
		});
	}
}
