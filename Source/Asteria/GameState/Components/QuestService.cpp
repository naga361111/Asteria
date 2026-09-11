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
	DOREPLIFETIME(UQuestService, Claims);
}

void UQuestService::OnRep_QuestPull()
{
	OnQuestPullChanged.Broadcast();
}

void UQuestService::OnRep_Claims()
{
	OnClaimsChanged.Broadcast();
}

int32 UQuestService::ClaimQuest(int32 QuestId, const TArray<int32>& Party)
{
	// 소유·복제 방향 불변조건: 상태 변경은 호스트만.
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return INDEX_NONE;
	}

	// 입력 검증(경계): 존재하는가 / 아직 안 집혔는가 / 파티가 비지 않았는가.
	const bool bQuestExists = QuestPull.ContainsByPredicate(
		[QuestId](const FQuest& Q) { return Q.QuestId == QuestId; });
	if (!bQuestExists || IsQuestClaimed(QuestId) || Party.Num() == 0)
	{
		return INDEX_NONE;
	}

	FQuestClaim Claim;
	Claim.ClaimId = ClaimCount++;
	Claim.QuestId = QuestId;
	Claim.Party = Party;

	Claims.Add(Claim);

	// 서버는 OnRep이 자동 호출되지 않으므로 직접 통지.
	OnClaimsChanged.Broadcast();

	return Claim.ClaimId;
}

bool UQuestService::IsQuestClaimed(int32 QuestId) const
{
	return Claims.ContainsByPredicate(
		[QuestId](const FQuestClaim& C) { return C.QuestId == QuestId; });
}

int32 UQuestService::FindAvailableQuestId() const
{
	const FQuest* Quest = QuestPull.FindByPredicate(
		[this](const FQuest& Q) { return !IsQuestClaimed(Q.QuestId); });
	return Quest ? Quest->QuestId : INDEX_NONE;
}
