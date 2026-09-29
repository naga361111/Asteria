// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_DeleteQuest.generated.h"

/**
 * 내 NpcId가 들고 있는 Assignment를 진행 상태(Assigned~SettleConfirmed)와 관계없이 전부 지우고 바로 끝난다.
 * 지울 것이 없으면(퀘스트 없이 떠나는 경우) 뒤따르는 떠나기를 막지 않도록 Succeeded.
 */
UCLASS()
class ASTERIA_API UBTTask_DeleteQuest : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_DeleteQuest();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
