// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_WaitGameTime.h"

#include "Time/AsteriaTimeSubsystem.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Engine/World.h"

namespace
{
	/** per-실행 상태 — 대기가 끝나는 누적 게임 분. */
	struct FWaitGameTimeMemory
	{
		double EndMinute = 0.0;
	};
}

UBTTask_WaitGameTime::UBTTask_WaitGameTime()
{
	NodeName = TEXT("Wait (Game Time)");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_WaitGameTime::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (WaitMinutes <= 0.0f)
	{
		return EBTNodeResult::Succeeded;
	}

	// 서브시스템은 항상 존재 가정(월드 서브시스템).
	const UAsteriaTimeSubsystem* Time = OwnerComp.GetWorld()->GetSubsystem<UAsteriaTimeSubsystem>();
	FWaitGameTimeMemory* Mem = reinterpret_cast<FWaitGameTimeMemory*>(NodeMemory);
	Mem->EndMinute = Time->GetTimeMinutes() + WaitMinutes;
	return EBTNodeResult::InProgress;
}

void UBTTask_WaitGameTime::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	const UAsteriaTimeSubsystem* Time = OwnerComp.GetWorld()->GetSubsystem<UAsteriaTimeSubsystem>();
	const FWaitGameTimeMemory* Mem = reinterpret_cast<const FWaitGameTimeMemory*>(NodeMemory);
	if (Time->GetTimeMinutes() >= Mem->EndMinute)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

uint16 UBTTask_WaitGameTime::GetInstanceMemorySize() const
{
	return sizeof(FWaitGameTimeMemory);
}
