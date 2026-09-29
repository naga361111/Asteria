// Fill out your copyright notice in the Description page of Project Settings.


#include "CounterService.h"

#include "QuestService.h"
#include "GuildService.h"
#include "Data/GuildReputationData.h"
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

UGuildService* UCounterService::GetGuildService() const
{
	AAsteriaGameState* GameState = Cast<AAsteriaGameState>(GetOwner());
	return GameState ? GameState->GuildService : nullptr;
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

bool UCounterService::ReceiveQuestAssignment(int32 AssignmentId)
{
	UQuestService* QuestService = GetQuestService();
	return QuestService && QuestService->ReceiveQuestAssignment(AssignmentId);
}

bool UCounterService::SettleQuestAssignment(int32 AssignmentId)
{
	UQuestService* QuestService = GetQuestService();
	return QuestService && QuestService->SettleQuestAssignment(AssignmentId);
}

bool UCounterService::ReceiveSettleQuestAssignment(int32 AssignmentId)
{
	UQuestService* QuestService = GetQuestService();
	UGuildService* GuildService = GetGuildService();
	if (QuestService == nullptr || GuildService == nullptr) return false;

	// 수령하면 Assignment와 퀘스트가 제거된다 → 지급 근거(퀘스트 정의·명성 표 항목)는 수령 전에 모두 찾아 계산해 둔다.
	const FQuestAssignment* Assignment = QuestService->FindQuestAssignment(AssignmentId);
	if (Assignment == nullptr || Assignment->State != EQuestAssignmentState::SettleConfirmed) return false;

	// 실패한 퀘스트는 수령만 하고 적립하지 않는다 — 지급 근거(명성 표)가 필요 없으니 조회하지 않는다.
	// 값으로 복사해 둔다 — 수령 뒤 위 포인터는 무효.
	const bool bQuestFailed = Assignment->bQuestFailed;
	if (bQuestFailed) return QuestService->ReceiveSettleQuestAssignment(AssignmentId);

	const int32 QuestId = Assignment->QuestId;
	const FQuest* Quest = QuestService->QuestPull.FindByPredicate(
		[QuestId](const FQuest& Q) { return Q.QuestId == QuestId; });
	if (Quest == nullptr) return false;

	// 얻을 명성은 퀘스트 등급으로 명성 표에서 조회한다. 표가 없거나 등급이 빠져 있으면 적립할 값을 알 수 없다.
	if (GuildService->ReputationData == nullptr) return false;
	const int32* Reputation = GuildService->ReputationData->ReputationByQuestRank.Find(Quest->QuestRnk);
	if (Reputation == nullptr) return false;

	// 값으로 복사해 둔다 — 수령 뒤 위 포인터들은 무효.
	const int32 Commission = FMath::RoundToInt(Quest->RewardAmount * Quest->CommissionRate);
	const int32 ReputationGain = *Reputation;

	// 수령이 성공한 뒤에만 지급 — 거절된 수령에 지급하거나 중복 지급하지 않는다.
	if (!QuestService->ReceiveSettleQuestAssignment(AssignmentId)) return false;

	// 수수료 입금과 명성 적립은 길드에만 한다. NPC에게는 아무것도 주지 않는다.
	GuildService->AddGuildFunds(Commission);
	GuildService->AddGuildReputation(ReputationGain);
	return true;
}

bool UCounterService::IsQuestAssignmentSubmitted(int32 AssignmentId) const
{
	const UQuestService* QuestService = GetQuestService();
	const FQuestAssignment* Assignment = QuestService ? QuestService->FindQuestAssignment(AssignmentId) : nullptr;
	return Assignment && Assignment->State == EQuestAssignmentState::Submitted;
}
