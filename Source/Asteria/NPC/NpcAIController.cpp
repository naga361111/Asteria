// Copyright Epic Games, Inc. All Rights Reserved.

#include "NpcAIController.h"
#include "UtilityBrainComponent.h"
#include "ActionRunnerComponent.h"

ANpcAIController::ANpcAIController()
{
	Brain = CreateDefaultSubobject<UUtilityBrainComponent>(TEXT("Brain"));
	ActionRunner = CreateDefaultSubobject<UActionRunnerComponent>(TEXT("ActionRunner"));

	// 브레인이 실행을 넘길 대상 연결. (넘기는 호출은 아직 구현 안 함)
	Brain->ActionRunner = ActionRunner;
}
