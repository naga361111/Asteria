// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/AsteriaGameState.h"
#include "Quest/Quest.h"
#include "Net/UnrealNetwork.h"

void AAsteriaGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAsteriaGameState, QuestPulls)
	DOREPLIFETIME(AAsteriaGameState, GuildMoney)
}

void AAsteriaGameState::OnRep_QuestPulls()
{
	OnQuestPullsChanged.Broadcast();
}

void AAsteriaGameState::OnRep_GuildBank()
{
	OnGuildMoneyChanged.Broadcast(GuildMoney);
}

void AAsteriaGameState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		int32 QuestIndex = 0;
		for (int i = 0; i < QuestPullsCount; ++i)
		{
			const int32 LastIndex = StaticEnum<ERank>()->NumEnums() - 3; // -1: _MAX, -1: 인덱스 보정
			ERank RandomRank = static_cast<ERank>(FMath::RandRange(0, LastIndex));
			int32 RandomRewardAmount = FMath::RandRange(10, 100);

			FQuest Quest = {QuestIndex, EQuestType::Generated, RandomRank, RandomRewardAmount};
			QuestPulls.Add(Quest);

			QuestIndex++;
		}
		OnQuestPullsChanged.Broadcast();
	}
}

void AAsteriaGameState::PostQuest(int32 QuestId)
{
	if (!HasAuthority()) return;

	FQuest* Found = QuestPulls.FindByPredicate([QuestId](const FQuest& Quest) { return Quest.QuestId == QuestId; });
	if (Found != nullptr)
	{
		Found->QuestType = EQuestType::Posted;
	}
	OnQuestPullsChanged.Broadcast();
}

void AAsteriaGameState::UnpostQuest(int32 QuestId)
{
	if (!HasAuthority()) return;

	FQuest* Found = QuestPulls.FindByPredicate([QuestId](const FQuest& Quest) { return Quest.QuestId == QuestId; });
	if (Found != nullptr)
	{
		Found->QuestType = EQuestType::Generated;
	}
	OnQuestPullsChanged.Broadcast();
}

void AAsteriaGameState::AcceptQuest(TArray<int32> QuestId)
{
	if (!HasAuthority()) return;
	
	for (int32 Quest : QuestId)
	{
		QuestPulls[Quest].QuestType = EQuestType::Accepted;
	}
	
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, FString::Printf(TEXT("Success")));
}

int32 AAsteriaGameState::SelectQuest(ERank CurrentNpcRank)
{
	if (QuestPulls.Num() == 0) return -1;

	for (FQuest& Quest : QuestPulls)
	{
		if (Quest.QuestType == EQuestType::Posted && Quest.RecommendedRank <= CurrentNpcRank)
		{
			Quest.QuestType = EQuestType::Selected;

			OnQuestPullsChanged.Broadcast();
			return Quest.QuestId;
		}
	}

	OnQuestPullsChanged.Broadcast();
	return -1;
}

void AAsteriaGameState::ClearQuest(int32 QuestId)
{
	if (!HasAuthority()) return;

	FQuest* Found = QuestPulls.FindByPredicate([QuestId](const FQuest& Quest) { return Quest.QuestId == QuestId; });
	if (Found != nullptr)
	{
		Found->QuestType = EQuestType::Cleared;
	}

	OnQuestPullsChanged.Broadcast();
}

void AAsteriaGameState::AddGuildMoney(int32 Amount)
{
	if (!HasAuthority()) return;

	GuildMoney += Amount;
	
	OnGuildMoneyChanged.Broadcast(GuildMoney);
}
