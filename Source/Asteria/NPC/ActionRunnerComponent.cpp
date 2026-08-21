// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionRunnerComponent.h"

#include "NpcActionSet.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UActionRunnerComponent::UActionRunnerComponent()
{
	// 이벤트 구동(브레인이 RunSet 호출) — 틱 불필요.
	PrimaryComponentTick.bCanEverTick = false;
}

void UActionRunnerComponent::RunSet(UNpcActionSet* Set, const FVector& Target)
{
	AAIController* AI = Cast<AAIController>(GetOwner());
	if (AI == nullptr)
	{
		return;
	}

	UBehaviorTree* First = (Set != nullptr && Set->Behaviors.Num() > 0) ? Set->Behaviors[0] : nullptr;
	if (First == nullptr)
	{
		// Idle: 실행할 게 없으면 트리 정지(완전 idle).
		if (UBrainComponent* Brain = AI->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("ActionRunner: empty set"));
		}
		return;
	}

	AI->RunBehaviorTree(First);
	if (UBlackboardComponent* BB = AI->GetBlackboardComponent())
	{
		BB->SetValueAsVector(MoveGoalKey, Target);
	}
}
