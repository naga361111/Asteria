// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_StopLogic.generated.h"

/** BT 로직을 정지시켜 트리의 자동 재시작(루프)을 막는다. "한 번만" 실행할 행동 시퀀스의 마지막 노드로 둔다.
 *  다시 돌리려면 브레인이 RunBehaviorTree를 새로 호출해야 함 — 행동이 바뀔 때 그렇게 된다.
 *  반복 행동은 이 노드를 두지 않으면 기존대로 루프한다. */
UCLASS()
class ASTERIA_API UBTTask_StopLogic : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_StopLogic();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
