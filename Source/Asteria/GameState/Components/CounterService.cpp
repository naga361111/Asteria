// Fill out your copyright notice in the Description page of Project Settings.


#include "CounterService.h"

#include "QuestService.h"
#include "GameState/AsteriaGameState.h"


// Sets default values for this component's properties
UCounterService::UCounterService()
{
	// 복제할 상태가 없다. 제출함은 QuestService의 QuestAssignments에서 파생되는 뷰일 뿐.
	PrimaryComponentTick.bCanEverTick = false;
}

UQuestService* UCounterService::GetQuestService() const
{
	AAsteriaGameState* GameState = Cast<AAsteriaGameState>(GetOwner());
	return GameState ? GameState->QuestService : nullptr;
}

bool UCounterService::SubmitQuestAssignment(int32 AssignmentId)
{
	UQuestService* QuestService = GetQuestService();
	return QuestService && QuestService->SubmitQuestAssignment(AssignmentId);
}

bool UCounterService::SubmitForSettleQuestAssignment(int32 AssignmentId)
{
	UQuestService* QuestService = GetQuestService();
	return QuestService && QuestService->SubmitForSettleQuestAssignment(AssignmentId);
}

bool UCounterService::AcceptQuestAssignment(int32 AssignmentId)
{
	UQuestService* QuestService = GetQuestService();
	return QuestService && QuestService->AcceptQuestAssignment(AssignmentId);
}

bool UCounterService::SettleQuestAssignment(int32 AssignmentId)
{
	UQuestService* QuestService = GetQuestService();
	return QuestService && QuestService->SettleQuestAssignment(AssignmentId);
}

bool UCounterService::IsQuestAssignmentSubmitted(int32 AssignmentId) const
{
	const UQuestService* QuestService = GetQuestService();
	const FQuestAssignment* Assignment = QuestService ? QuestService->FindQuestAssignment(AssignmentId) : nullptr;
	return Assignment && Assignment->State == EQuestAssignmentState::Submitted;
}
