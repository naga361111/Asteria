// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NpcAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

void ANpcAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	TArray<AActor*> PostQuestBoardActor;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("PostQuestBoard"), PostQuestBoardActor);
	
	TArray<AActor*> NpcHomeActor;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("NpcHome"), NpcHomeActor);
	
	if (BehaviorTree != nullptr)
	{
		RunBehaviorTree(BehaviorTree);
		
		if (PostQuestBoardActor.Num() != 0)
		{
			GetBlackboardComponent()->SetValueAsObject(FName("PostQuestBoard"), PostQuestBoardActor[0]);
		}
		
		if (NpcHomeActor.Num() != 0)
		{
			GetBlackboardComponent()->SetValueAsObject(FName("NpcHome"), NpcHomeActor[0]);

		}
	}
}
