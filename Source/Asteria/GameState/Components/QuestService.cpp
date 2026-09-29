// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestService.h"

#include "GuildService.h"
#include "GameClockService.h"
#include "Data/QuestIssueData.h"
#include "GameState/AsteriaGameState.h"
#include "Net/UnrealNetwork.h"

namespace
{
	// 등급별 보상 액수 범위(최소~최대). ERank 순서(F..S)와 인덱스가 일치해야 한다.
	constexpr int32 RewardRangeByRank[static_cast<int32>(ERank::S) + 1][2] = {
		{ 10, 30 },     // F
		{ 30, 80 },     // E
		{ 80, 200 },    // D
		{ 200, 500 },   // C
		{ 500, 1200 },  // B
		{ 1200, 3000 }, // A
		{ 3000, 8000 }, // S
	};

	// 퀘스트마다 이 범위에서 수수료 비율을 뽑는다.
	constexpr float MinCommissionRate = 0.1f;
	constexpr float MaxCommissionRate = 0.3f;

	// 발행·만료를 확인하는 실제 시간 주기(초).
	constexpr float QuestClockIntervalSeconds = 1.f;

	// 반환 시 퀘스트를 보드에 되돌릴 최소 남은 시간(게임 분). 이보다 적게 남았으면 퀘스트를 지운다.
	constexpr int32 MinReturnRemainingMinutes = 5;

	// 길드 등급 기준 가중 추첨: 동급 1, 아래는 단계마다 LowerRankFalloff 배, 위 한 단계만 UpperRankWeight.
	ERank RollQuestRank(ERank GuildRank)
	{
		// 길드 등급에서 한 단계 내려갈 때마다 곱하는 무게 비율.
		constexpr float LowerRankFalloff = 0.5f;
		// 길드 등급 바로 위 한 단계의 무게. 그보다 위는 발행하지 않는다.
		constexpr float UpperRankWeight = 0.3f;

		const int32 Guild = static_cast<int32>(GuildRank);
		const int32 Top = FMath::Min(Guild + 1, static_cast<int32>(ERank::S)); // S급 초과는 없다

		float Weight[static_cast<int32>(ERank::S) + 1] = {};
		float TotalWeight = 0.f;
		for (int32 R = 0; R <= Top; ++R)
		{
			Weight[R] = R > Guild ? UpperRankWeight : FMath::Pow(LowerRankFalloff, static_cast<float>(Guild - R));
			TotalWeight += Weight[R];
		}

		// PickedRnk를 매 등급마다 갱신해 FP 잔차로 아무것도 안 뽑히는 걸 막는다.
		float Roll = FMath::FRand() * TotalWeight;
		int32 PickedRnk = 0;
		for (int32 R = 0; R <= Top; ++R)
		{
			PickedRnk = R;
			Roll -= Weight[R];
			if (Roll < 0.f)
			{
				break;
			}
		}
		return static_cast<ERank>(PickedRnk);
	}
}

// Sets default values for this component's properties
UQuestService::UQuestService()
{
	SetIsReplicatedByDefault(true);
}

void UQuestService::BeginPlay()
{
	Super::BeginPlay();

	// 소유·복제 방향 불변조건: 발행은 호스트만. 클라는 복제된 QuestPull만 받는다.
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	const AAsteriaGameState* GameState = Cast<AAsteriaGameState>(GetOwner());
	if (!GameState || !GameState->GuildService || !GameState->GameClockService || !IssueData)
	{
		UE_LOG(LogTemp, Warning, TEXT("QuestService: no owning GameState, GuildService, GameClockService or IssueData. Quests not generated."));
		return;
	}

	// 첫 묶음은 즉시 발행하고, 다음 예정 시각은 지금부터 한 주기 뒤.
	// 시작 등급 항목이 없으면 예정 시각이 지금에 머물러, 항목이 있는 등급이 되는 순간 발행한다.
	NextBatchSlotMinute = NextBatchMinute = GameState->GameClockService->GetGameMinutes();
	if (const FQuestIssueSettings* Settings = FindIssueSettings())
	{
		IssueQuestBatch(*Settings);
	}

	GetWorld()->GetTimerManager().SetTimer(QuestClockTimer, this, &UQuestService::TickQuestClock,
		QuestClockIntervalSeconds, true);

	// 서버는 OnRep이 자동 호출되지 않으므로 직접 통지.
	OnQuestPullChanged.Broadcast();
}

const FQuestIssueSettings* UQuestService::FindIssueSettings() const
{
	const AAsteriaGameState* GameState = Cast<AAsteriaGameState>(GetOwner());
	if (!IssueData || !GameState || !GameState->GuildService)
	{
		return nullptr;
	}
	return IssueData->IssueSettingsByGuildRank.Find(GameState->GuildService->GuildRank);
}

void UQuestService::IssueQuestBatch(const FQuestIssueSettings& Settings)
{
	const AAsteriaGameState* GameState = Cast<AAsteriaGameState>(GetOwner());
	if (!GameState || !GameState->GuildService || !GameState->GameClockService)
	{
		return;
	}

	// 등급·설정은 발행 시점의 길드 등급 기준 — 길드가 성장하면 이후 묶음부터 반영된다.
	const ERank GuildRank = GameState->GuildService->GuildRank;
	const int32 Now = GameState->GameClockService->GetGameMinutes();
	const int32 BatchSize = FMath::RandRange(Settings.MinBatchSize, Settings.MaxBatchSize);
	for (int32 i = 0; i < BatchSize; ++i)
	{
		FQuest Quest;
		Quest.QuestId = QuestCount++;
		Quest.QuestRnk = RollQuestRank(GuildRank);
		const int32 Rnk = static_cast<int32>(Quest.QuestRnk);
		Quest.RewardAmount = FMath::RandRange(RewardRangeByRank[Rnk][0], RewardRangeByRank[Rnk][1]);
		Quest.CommissionRate = FMath::FRandRange(MinCommissionRate, MaxCommissionRate);
		Quest.ExpireGameMinute = Now + FMath::RandRange(Settings.MinQuestLifetimeMinutes, Settings.MaxQuestLifetimeMinutes);
		QuestPull.Add(Quest);
	}

	// 기준 시각만 한 주기 전진시켜 흔들림이 다음 예정 시각에 누적되지 않게 한다.
	// 표 항목이 없어 발행을 쉬었다면 기준 시각이 한참 뒤처져 있으니, 밀린 묶음을 연달아 내지 않도록 지금 근처로 당긴다.
	NextBatchSlotMinute = FMath::Max(NextBatchSlotMinute, Now - Settings.BatchJitterMinutes) + Settings.BatchPeriodMinutes;
	NextBatchMinute = NextBatchSlotMinute + FMath::RandRange(-Settings.BatchJitterMinutes, Settings.BatchJitterMinutes);
}

void UQuestService::TickQuestClock()
{
	const AAsteriaGameState* GameState = Cast<AAsteriaGameState>(GetOwner());
	if (!GameState || !GameState->GameClockService)
	{
		return;
	}

	const int32 Now = GameState->GameClockService->GetGameMinutes();
	bool bChanged = false;

	if (Now >= NextBatchMinute)
	{
		if (const FQuestIssueSettings* Settings = FindIssueSettings())
		{
			IssueQuestBatch(*Settings);
			bChanged = true;
		}
	}

	// NPC가 집은 퀘스트는 만료 시각이 지나도 남긴다 — 진행 중인 Assignment가 가리키는 대상이 사라지면 안 된다.
	if (QuestPull.RemoveAll([this, Now](const FQuest& Q)
		{ return Now >= Q.ExpireGameMinute && !IsQuestAssigned(Q.QuestId); }) > 0)
	{
		bChanged = true;
	}

	// 서버는 OnRep이 자동 호출되지 않으므로 직접 통지.
	if (bChanged)
	{
		OnQuestPullChanged.Broadcast();
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

bool UQuestService::ReceiveQuestAssignment(int32 AssignmentId)
{
	return TransitionQuestAssignment(AssignmentId,
		EQuestAssignmentState::Accepted, EQuestAssignmentState::Received) != nullptr;
}

bool UQuestService::ClearQuestAssignment(int32 AssignmentId)
{
	return TransitionQuestAssignment(AssignmentId,
		EQuestAssignmentState::Received, EQuestAssignmentState::Cleared) != nullptr;
}

bool UQuestService::SubmitForSettleQuestAssignment(int32 AssignmentId)
{
	return TransitionQuestAssignment(AssignmentId,
		EQuestAssignmentState::Cleared, EQuestAssignmentState::SubmitForSettled) != nullptr;
}

bool UQuestService::SettleQuestAssignment(int32 AssignmentId)
{
	if (TransitionQuestAssignment(AssignmentId,
		EQuestAssignmentState::SubmitForSettled, EQuestAssignmentState::SettleConfirmed) == nullptr)
	{
		return false;
	}

	// 정산 컨펌을 기다리며 멈춰 있는 NPC를 깨운다. 서버 로컬 신호라 여기서만 발화한다.
	// (AcceptQuestAssignment가 OnQuestAssignmentAccepted를 쏘는 것과 같은 자리)
	OnQuestAssignmentSettled.Broadcast(AssignmentId);

	return true;
}

bool UQuestService::ReceiveSettleQuestAssignment(int32 AssignmentId)
{
	if (TransitionQuestAssignment(AssignmentId,
		EQuestAssignmentState::SettleConfirmed, EQuestAssignmentState::Settled) == nullptr)
	{
		return false;
	}

	// 수령으로 정산이 끝났으니 Assignment와 퀘스트를 배열에서 지운다.
	// 지급 근거는 호출자(CounterService)가 수령 전에 미리 읽어 둔다.
	return DeleteQuestAssignment(AssignmentId);
}

bool UQuestService::DeleteQuestAssignment(int32 AssignmentId)
{
	// 소유·복제 방향 불변조건: 상태 변경은 호스트만.
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return false;
	}

	const FQuestAssignment* Assignment = FindQuestAssignment(AssignmentId);
	if (!Assignment)
	{
		return false;
	}
	// 반환 포인터는 배열이 바뀌면 무효 — 제거 전에 QuestId를 복사해 둔다.
	const int32 QuestId = Assignment->QuestId;

	if (QuestAssignments.RemoveAll(
		[AssignmentId](const FQuestAssignment& C) { return C.AssignmentId == AssignmentId; }) > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Assignment Removed: %d"), AssignmentId);
	}
	if (QuestPull.RemoveAll(
		[QuestId](const FQuest& Q) { return Q.QuestId == QuestId; }) > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Quest Removed: %d"), QuestId);
	}

	// 서버는 OnRep이 자동 호출되지 않으므로 직접 통지.
	OnQuestAssignmentsChanged.Broadcast();
	OnQuestPullChanged.Broadcast();

	return true;
}

bool UQuestService::ReturnQuestAssignment(int32 AssignmentId)
{
	// 소유·복제 방향 불변조건: 상태 변경은 호스트만.
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return false;
	}

	// 수주 단계만 반환 가능 — Received 이후는 수행·정산 중이라 보드로 되돌리면 안 된다.
	const FQuestAssignment* Assignment = FindQuestAssignment(AssignmentId);
	if (!Assignment
		|| (Assignment->State != EQuestAssignmentState::Assigned
			&& Assignment->State != EQuestAssignmentState::Submitted
			&& Assignment->State != EQuestAssignmentState::Accepted))
	{
		return false;
	}

	// 남은 시간을 판정할 수 없으면 아무것도 지우지 않는다.
	const AAsteriaGameState* GameState = Cast<AAsteriaGameState>(GetOwner());
	if (!GameState || !GameState->GameClockService)
	{
		return false;
	}
	const int32 Now = GameState->GameClockService->GetGameMinutes();

	// 반환 포인터는 배열이 바뀌면 무효 — 제거 전에 QuestId를 복사해 둔다.
	const int32 QuestId = Assignment->QuestId;

	QuestAssignments.RemoveAll(
		[AssignmentId](const FQuestAssignment& C) { return C.AssignmentId == AssignmentId; });

	// 만료가 코앞이면 보드에 되돌려 봐야 곧 사라지니 퀘스트째 지운다.
	const bool bQuestRemoved = QuestPull.RemoveAll([QuestId, Now](const FQuest& Q)
		{ return Q.QuestId == QuestId && Q.ExpireGameMinute - Now < MinReturnRemainingMinutes; }) > 0;

	// 서버는 OnRep이 자동 호출되지 않으므로 직접 통지.
	OnQuestAssignmentsChanged.Broadcast();
	if (bQuestRemoved)
	{
		OnQuestPullChanged.Broadcast();
	}

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
