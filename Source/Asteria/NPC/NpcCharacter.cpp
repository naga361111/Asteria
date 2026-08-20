// Copyright Epic Games, Inc. All Rights Reserved.

#include "NpcCharacter.h"
#include "NpcAIController.h"
#include "NeedsComponent.h"

ANpcCharacter::ANpcCharacter()
{
	// 배치/스폰되면 자기 AIController가 자동으로 빙의한다.
	AIControllerClass = ANpcAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	Needs = CreateDefaultSubobject<UNeedsComponent>(TEXT("Needs"));
}
