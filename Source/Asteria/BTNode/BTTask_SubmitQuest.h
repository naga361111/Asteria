// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SubmitQuest.generated.h"

/**
 * 내 NpcId가 들고 있는 Assigned Assignment를 창구에 제출(Assigned→Submitted)하고 바로 끝난다.
 * 컨펌 대기는 뒤따르는 Wait For Confirm Quest가 맡는다.
 */
UCLASS()
class ASTERIA_API UBTTask_SubmitQuest : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SubmitQuest();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
