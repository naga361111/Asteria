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
		Quest.QuestRnk = static_cast<ERank>(FMath::RandRange(0, static_cast<int32>(ERank::S)));
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

bool UQuestService::SubmitForSettleQuestAssignment(int32 AssignmentId)
{
	return TransitionQuestAssignment(AssignmentId,
		EQuestAssignmentState::Cleared, EQuestAssignmentState::SubmitForSettled) != nullptr;
}

bool UQuestService::SettleQuestAssignment(int32 AssignmentId)
{
	if (TransitionQuestAssignment(AssignmentId,
		EQuestAssignmentState::SubmitForSettled, EQuestAssignmentState::Settled) == nullptr)
	{
		return false;
	}

	// 정산을 기다리며 멈춰 있는 NPC를 깨운다. 서버 로컬 신호라 여기서만 발화한다.
	// (AcceptQuestAssignment가 OnQuestAssignmentAccepted를 쏘는 것과 같은 자리)
	OnQuestAssignmentSettled.Broadcast(AssignmentId);

	return true;
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

int32 UQuestService::FindAvailableQuestId(ERank MaxRank) const
{
	const int32 Max = static_cast<int32>(MaxRank);

	UE_LOG(LogTemp, Warning, TEXT("FindAvailableQuestId ENTER: MaxRank=%d"), Max);

	// 1패스: 가용 퀘스트가 있는 등급마다 가중치를 딱 한 번 부여한다.
	// 등급 단위로 확률을 매기는 게 핵심 — 이래야 아래 등급 퀘스트가 아무리 많아도
	// 그 등급의 당첨 확률이 고정되어 보드 구성에 휘둘리지 않는다.
	// ponytail: RankFalloff 하나가 꼬리 두께. 가파르면 거의 동급, 완만하면 아래도 자주.
	constexpr float RankFalloff = 0.2f;
	float TierWeight[static_cast<int32>(ERank::S) + 1] = {};
	float TotalWeight = 0.f;
	for (const FQuest& Q : QuestPull)
	{
		const int32 Rnk = static_cast<int32>(Q.QuestRnk);
		if (Rnk > Max || IsQuestAssigned(Q.QuestId)) // 천장 + 이미 집힘
		{
			continue;
		}
		if (TierWeight[Rnk] == 0.f) // 이 등급 첫 발견 시에만
		{
			TierWeight[Rnk] = FMath::Pow(RankFalloff, static_cast<float>(Max - Rnk));
			TotalWeight += TierWeight[Rnk];
		}
	}
	if (TotalWeight == 0.f)
	{
		return INDEX_NONE;
	}

	// 등급 가중 추첨. PickedRnk를 매 가용 등급마다 갱신해 FP 잔차로 아무것도 안 뽑히는 걸 막는다.
	float Roll = FMath::FRand() * TotalWeight;
	int32 PickedRnk = INDEX_NONE;
	for (int32 R = 0; R <= static_cast<int32>(ERank::S); ++R)
	{
		if (TierWeight[R] == 0.f)
		{
			continue;
		}
		PickedRnk = R;
		Roll -= TierWeight[R];
		if (Roll < 0.f)
		{
			break;
		}
	}

	// 2패스: 뽑힌 등급 안에서 균등 랜덤(리저버 — 임시배열 없이 1패스).
	int32 PickedId = INDEX_NONE;
	int32 Seen = 0;
	for (const FQuest& Q : QuestPull)
	{
		if (static_cast<int32>(Q.QuestRnk) != PickedRnk || IsQuestAssigned(Q.QuestId))
		{
			continue;
		}
		if (FMath::RandRange(0, Seen++) == 0)
		{
			PickedId = Q.QuestId;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("FindAvailableQuestId EXIT: PickedRnk=%d PickedId=%d"), PickedRnk, PickedId);

	return PickedId;
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
