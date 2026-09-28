// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ReceiveSettle.generated.h"

/**
 * 내 NpcId의 SettleConfirmed Assignment를 창구에서 수령(SettleConfirmed→Settled)하고 바로 끝난다.
 * 수령 순간 서버에서 지급(수수료·명성)되고 Assignment와 퀘스트가 제거된다.
 */
UCLASS()
class ASTERIA_API UBTTask_ReceiveSettle : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ReceiveSettle();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
