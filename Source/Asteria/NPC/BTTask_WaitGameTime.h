// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_WaitGameTime.generated.h"

/** 게임 시간 기준 대기 — WaitMinutes(게임 분)만큼 지나면 Succeeded.
 *  게임 시간(AsteriaTimeSubsystem)으로 재므로 pause 시 함께 멈춘다.
 *  WaitMinutes <= 0 이면 즉시 성공. 대기 상태는 NodeMemory에 저장(인스턴스 공유 안전). */
UCLASS()
class ASTERIA_API UBTTask_WaitGameTime : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_WaitGameTime();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;

protected:
	/** 대기할 게임 분. 노드에서 설정. */
	UPROPERTY(EditAnywhere, Category="Wait", meta=(ClampMin="0"))
	float WaitMinutes = 60.0f;
};
