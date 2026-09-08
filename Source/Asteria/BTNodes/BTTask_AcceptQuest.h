// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_AcceptQuest.generated.h"

/**
 * 
 */
UCLASS()
class ASTERIA_API UBTTask_AcceptQuest : public UBTTaskNode
{
	GENERATED_BODY()
	
	UBTTask_AcceptQuest();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
	
	void HandleQuestAccept();
};
