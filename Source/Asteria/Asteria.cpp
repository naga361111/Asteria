// Copyright Epic Games, Inc. All Rights Reserved.

#include "Asteria.h"
#include "Modules/ModuleManager.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebugger.h"
#include "Debugger/GameplayDebuggerCategoryQuest.h"
#endif

class FAsteriaModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override
	{
#if WITH_GAMEPLAY_DEBUGGER
		IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
		GameplayDebuggerModule.RegisterCategory(
			"Quest",
			IGameplayDebugger::FOnGetCategory::CreateStatic(&FGameplayDebuggerCategoryQuest::MakeInstance),
			EGameplayDebuggerCategoryState::EnabledInGameAndSimulate);
		GameplayDebuggerModule.NotifyCategoriesChanged();
#endif
	}

	virtual void ShutdownModule() override
	{
#if WITH_GAMEPLAY_DEBUGGER
		if (IGameplayDebugger::IsAvailable())
		{
			IGameplayDebugger& GameplayDebuggerModule = IGameplayDebugger::Get();
			GameplayDebuggerModule.UnregisterCategory("Quest");
			GameplayDebuggerModule.NotifyCategoriesChanged();
		}
#endif
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE( FAsteriaModule, Asteria, "Asteria" );
