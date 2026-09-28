// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SubmitSettle.generated.h"

/**
 * 내 NpcId가 들고 있는 Cleared Assignment를 창구의 보상 대기 제출함에 올리고(Cleared→SubmitForSettled) 바로 끝난다.
 * 정산 컨펌 대기는 뒤따르는 Wait For Settle Confirm이 맡는다.
 */
UCLASS()
class ASTERIA_API UBTTask_SubmitSettle : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SubmitSettle();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
