// Copyright Epic Games, Inc. All Rights Reserved.

#include "NpcAIController.h"
#include "UtilityBrainComponent.h"

ANpcAIController::ANpcAIController()
{
	Brain = CreateDefaultSubobject<UUtilityBrainComponent>(TEXT("Brain"));
}
