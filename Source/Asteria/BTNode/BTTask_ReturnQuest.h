// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ReturnQuest.generated.h"

/**
 * 내 NpcId가 들고 있는 수주 단계(Submitted·Assigned·Accepted) Assignment를 반환하고 바로 끝난다.
 * 반환할 것이 없으면(퀘스트를 못 집고 떠나는 경우) 뒤따르는 떠나기를 막지 않도록 Succeeded.
 */
UCLASS()
class ASTERIA_API UBTTask_ReturnQuest : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ReturnQuest();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
