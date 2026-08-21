// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_StopLogic.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_StopLogic::UBTTask_StopLogic()
{
	NodeName = TEXT("Stop Logic");
}

EBTNodeResult::Type UBTTask_StopLogic::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* /*NodeMemory*/)
{
	// 트리 정지 → 완료 후 매 프레임 재시작(루프) 방지. 다음 행동은 브레인의 RunBehaviorTree가 다시 띄운다.
	OwnerComp.StopLogic(TEXT("Action finished (BTTask_StopLogic)"));
	return EBTNodeResult::Succeeded;
}
