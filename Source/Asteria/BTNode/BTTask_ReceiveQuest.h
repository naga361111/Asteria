// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ReceiveQuest.generated.h"

/**
 * 내 NpcId의 Accepted Assignment를 창구에서 회수(Accepted→Received)하고 바로 끝난다.
 * 회수해야 보드에서 빠지고, 뒤따르는 Do Quest가 Received를 찾아 수행한다.
 */
UCLASS()
class ASTERIA_API UBTTask_ReceiveQuest : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ReceiveQuest();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
