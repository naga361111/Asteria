// Fill out your copyright notice in the Description page of Project Settings.


#include "BTNodes/BTTask_GetQuest.h"

#include "GameState/AsteriaGameState.h"

EBTNodeResult::Type UBTTask_GetQuest::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAsteriaGameState* AGS = GetWorld()->GetGameState<AAsteriaGameState>();

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, FString::Printf(
		                                 TEXT("[%d]Accepted quest id: %d"), GetWorld()->GetNetMode(),
		                                 AGS->GetQuest()));

	return EBTNodeResult::Succeeded;
}
