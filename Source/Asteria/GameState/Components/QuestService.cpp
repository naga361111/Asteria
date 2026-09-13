// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestService.h"

#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UQuestService::UQuestService()
{
	SetIsReplicatedByDefault(true);

	for (int i = 0; i < 50; ++i)
	{
		FQuest Quest;
		Quest.QuestId = QuestCount++;
		QuestPull.Add(Quest);
	}
}

void UQuestService::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UQuestService, QuestPull);
	DOREPLIFETIME(UQuestService, QuestAssignments);
	DOREPLIFETIME(UQuestService, SettleQuestAssignments);
}

void UQuestService::OnRep_QuestPull()
{
	OnQuestPullChanged.Broadcast();
}

void UQuestService::OnRep_QuestAssignments()
{
	OnQuestAssignmentsChanged.Broadcast();
}

int32 UQuestService::AssignQuest(int32 QuestId, const TArray<int32>& Party)
{
	UE_LOG(LogTemp, Warning, TEXT("AssignQuest Start: %d"), QuestAssignments.Num())
	
	// 소유·복제 방향 불변조건: 상태 변경은 호스트만.
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return INDEX_NONE;
	}

	// 입력 검증(경계): 존재하는가 / 아직 안 집혔는가 / 파티가 비지 않았는가.
	const bool bQuestExists = QuestPull.ContainsByPredicate(
		[QuestId](const FQuest& Q) { return Q.QuestId == QuestId; });
	if (!bQuestExists || IsQuestAssigned(QuestId) || Party.Num() == 0)
	{
		return INDEX_NONE;
	}

	FQuestAssignment Assignment;
	Assignment.AssignmentId = AssignmentCount++;
	Assignment.QuestId = QuestId;
	Assignment.Party = Party;
	Assignment.State = EQuestAssignmentState::Assigned;

	QuestAssignments.Add(Assignment);

	// 서버는 OnRep이 자동 호출되지 않으므로 직접 통지.
	OnQuestAssignmentsChanged.Broadcast();

	return Assignment.AssignmentId;
}

FQuestAssignment* UQuestService::TransitionQuestAssignment(int32 AssignmentId, EQuestAssignmentState From, EQuestAssignmentState To)
{
	// 소유·복제 방향 불변조건: 상태 변경은 호스트만.
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return nullptr;
	}

	// 입력 검증(경계): 실재하는 Assignment인가 / 지금이 From인가.
	// 후자가 중복 호출과 역행(예: Accepted→Submitted)을 동시에 막는다.
	FQuestAssignment* Assignment = QuestAssignments.FindByPredicate(
		[AssignmentId](const FQuestAssignment& C) { return C.AssignmentId == AssignmentId; });
	if (!Assignment || Assignment->State != From)
	{
		return nullptr;
	}

	Assignment->State = To;

	// 서버는 OnRep이 자동 호출되지 않으므로 직접 통지.
	OnQuestAssignmentsChanged.Broadcast();

	return Assignment;
}

bool UQuestService::SubmitQuestAssignment(int32 AssignmentId)
{
	return TransitionQuestAssignment(AssignmentId,
		EQuestAssignmentState::Assigned, EQuestAssignmentState::Submitted) != nullptr;
}

bool UQuestService::AcceptQuestAssignment(int32 AssignmentId)
{
	if (TransitionQuestAssignment(AssignmentId,
		EQuestAssignmentState::Submitted, EQuestAssignmentState::Accepted) == nullptr)
	{
		return false;
	}

	// 이 Assignment를 기다리며 멈춰 있는 NPC를 깨운다. 서버 로컬 신호라 여기서만 발화한다.
	OnQuestAssignmentAccepted.Broadcast(AssignmentId);

	return true;
}

bool UQuestService::ClearQuestAssignment(int32 AssignmentId)
{
	return TransitionQuestAssignment(AssignmentId,
		EQuestAssignmentState::Accepted, EQuestAssignmentState::Cleared) != nullptr;
}

bool UQuestService::EnqueueSettleQuestAssignment(int32 AssignmentId)
{
	// 소유·복제 방향 불변조건: 대기열 쓰기도 호스트만.
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return false;
	}

	// 정산 대기 자격은 원본의 State가 정한다 — 대기열은 그 사실을 복제해 보여주는 사본일 뿐이다.
	const FQuestAssignment* Assignment = FindQuestAssignment(AssignmentId);
	if (!Assignment || Assignment->State != EQuestAssignmentState::Cleared)
	{
		return false;
	}

	// 재진입(태스크 재실행 등)으로 같은 Assignment가 두 줄 서지 않게.
	const bool bAlreadyQueued = SettleQuestAssignments.ContainsByPredicate(
		[AssignmentId](const FQuestAssignment& C) { return C.AssignmentId == AssignmentId; });
	if (bAlreadyQueued)
	{
		return false;
	}

	SettleQuestAssignments.Add(*Assignment);

	return true;
}

bool UQuestService::DequeueSettleQuestAssignment(int32 AssignmentId)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return false;
	}

	return SettleQuestAssignments.RemoveAll(
		[AssignmentId](const FQuestAssignment& C) { return C.AssignmentId == AssignmentId; }) > 0;
}

bool UQuestService::IsQuestAssigned(int32 QuestId) const
{
	return QuestAssignments.ContainsByPredicate(
		[QuestId](const FQuestAssignment& C) { return C.QuestId == QuestId; });
}

int32 UQuestService::FindAvailableQuestId() const
{
	const FQuest* Quest = QuestPull.FindByPredicate(
		[this](const FQuest& Q) { return !IsQuestAssigned(Q.QuestId); });
	return Quest ? Quest->QuestId : INDEX_NONE;
}

const FQuestAssignment* UQuestService::FindQuestAssignment(int32 AssignmentId) const
{
	return QuestAssignments.FindByPredicate(
		[AssignmentId](const FQuestAssignment& C) { return C.AssignmentId == AssignmentId; });
}

const FQuestAssignment* UQuestService::FindQuestAssignmentByNpc(int32 NpcId, EQuestAssignmentState State) const
{
	return QuestAssignments.FindByPredicate(
		[NpcId, State](const FQuestAssignment& C) { return C.State == State && C.Party.Contains(NpcId); });
}
