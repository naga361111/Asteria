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
			FQuest Quest = {QuestIndex, false, false};
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
		Found->bIsPosted = true;
	}
	OnQuestPullsChanged.Broadcast();
}

void AAsteriaGameState::UnpostQuest(int32 QuestId)
{
	if (!HasAuthority()) return;

	FQuest* Found = QuestPulls.FindByPredicate([QuestId](const FQuest& Quest) { return Quest.QuestId == QuestId; });
	if (Found != nullptr)
	{
		Found->bIsPosted = false;
	}
	OnQuestPullsChanged.Broadcast();
}

int32 AAsteriaGameState::GetQuest()
{
	if (QuestPulls.Num() == 0) return -1;

	for (FQuest& Quest : QuestPulls)
	{
		if (Quest.bIsPosted && !Quest.bIsAccepted)
		{
			Quest.bIsAccepted = true;

			return Quest.QuestId;
		}
	}

	return -1;
}
