// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/AsteriaGameState.h"
#include "Quest/Quest.h"
#include "Net/UnrealNetwork.h"

void AAsteriaGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAsteriaGameState, QuestPulls)
}

void AAsteriaGameState::OnRep_QuestPulls()
{
	OnQuestPullsChanged.Broadcast();
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
			
			FQuest Quest = {QuestIndex, EQuestType::Generated, RandomRank};
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

int32 AAsteriaGameState::GetQuest(ERank CurrentNpcRank)
{
	if (QuestPulls.Num() == 0) return -1;

	for (FQuest& Quest : QuestPulls)
	{
		if (Quest.QuestType == EQuestType::Posted && Quest.RecommendedRank <= CurrentNpcRank)
		{
			Quest.QuestType = EQuestType::Accepted; // 추후 Npc의 선택과 유저의 확정 로직 분리

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
