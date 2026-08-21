// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Asteria : ModuleRules
{
	public Asteria(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// 모듈 루트를 include 경로에 추가 → 폴더 간 include를 "Place/Foo.h"처럼 쓴다.
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "AIModule" });

		PrivateDependencyModuleNames.AddRange(new string[] { "DeveloperSettings" });

		// GameplayDebugger 모듈 의존성 추가 + WITH_GAMEPLAY_DEBUGGER 정의(Shipping 제외).
		SetupGameplayDebuggerSupport(Target);

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
