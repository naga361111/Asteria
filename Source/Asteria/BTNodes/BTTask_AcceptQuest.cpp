// Fill out your copyright notice in the Description page of Project Settings.


#include "BTNodes/BTTask_AcceptQuest.h"
#include "AIController.h"
#include "NPC/AsteriaNpc.h"

UBTTask_AcceptQuest::UBTTask_AcceptQuest()
{
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_AcceptQuest::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAsteriaNpc* Npc = OwnerComp.GetAIOwner()->GetPawn<AAsteriaNpc>();
	if (Npc == nullptr) return EBTNodeResult::Failed;

	CachedOwnerComp = &OwnerComp;
	Npc->OnQuestAccepted.AddUObject(this, &UBTTask_AcceptQuest::HandleQuestAccept);
	Npc->bWaitForQuestAccepted = true;

	UE_LOG(LogTemp, Warning, TEXT("InProgress"))
	return EBTNodeResult::InProgress;
}

void UBTTask_AcceptQuest::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
                                         EBTNodeResult::Type TaskResult)
{
	AAsteriaNpc* Npc = OwnerComp.GetAIOwner()->GetPawn<AAsteriaNpc>();
	if (Npc != nullptr)
	{
		Npc->bWaitForQuestAccepted = false;
		Npc->OnQuestAccepted.RemoveAll(this);
		UE_LOG(LogTemp, Warning, TEXT("Exit"))
	}
}

void UBTTask_AcceptQuest::HandleQuestAccept()
{
	FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
}
