// Copyright Epic Games, Inc. All Rights Reserved.

#include "Asteria.h"
#include "Modules/ModuleManager.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebugger.h"
#include "NPC/NpcDebuggerCategory.h"
#endif

class FAsteriaModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override
	{
#if WITH_GAMEPLAY_DEBUGGER
		IGameplayDebugger& GameplayDebugger = IGameplayDebugger::Get();
		GameplayDebugger.RegisterCategory(
			"Asteria.NPC",
			IGameplayDebugger::FOnGetCategory::CreateStatic(&FNpcDebuggerCategory::MakeInstance),
			EGameplayDebuggerCategoryState::EnabledInGameAndSimulate);
		GameplayDebugger.NotifyCategoriesChanged();
#endif
	}

	virtual void ShutdownModule() override
	{
#if WITH_GAMEPLAY_DEBUGGER
		if (IGameplayDebugger::IsAvailable())
		{
			IGameplayDebugger& GameplayDebugger = IGameplayDebugger::Get();
			GameplayDebugger.UnregisterCategory("Asteria.NPC");
			GameplayDebugger.NotifyCategoriesChanged();
		}
#endif
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE( FAsteriaModule, Asteria, "Asteria" );
