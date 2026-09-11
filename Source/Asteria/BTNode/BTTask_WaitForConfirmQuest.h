// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_WaitForConfirmQuest.generated.h"

/**
 * 퀘스트 확정을 무한정 대기한다. 외부(서버 권위)에서 FinishLatentTask를 호출하기 전까지 끝나지 않는다.
 */
UCLASS()
class ASTERIA_API UBTTask_WaitForConfirmQuest : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_WaitForConfirmQuest();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
