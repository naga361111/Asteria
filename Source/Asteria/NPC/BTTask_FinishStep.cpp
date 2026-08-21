// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_FinishStep.h"

#include "ActionRunnerComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

EBTNodeResult::Type UBTTask_FinishStep::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AAIController* AI = OwnerComp.GetAIOwner())
	{
		if (UActionRunnerComponent* Runner = AI->FindComponentByClass<UActionRunnerComponent>())
		{
			Runner->Advance();
		}
	}
	return EBTNodeResult::Succeeded;
}
