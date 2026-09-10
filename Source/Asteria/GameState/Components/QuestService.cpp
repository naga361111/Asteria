// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestService.h"

#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UQuestService::UQuestService()
{
	SetIsReplicatedByDefault(true);

	for (int i = 0; i < 50; ++i)
	{
		FQuest Quest = {QuestCount, EQuest::Generated};
		QuestPull.Add(Quest);

		QuestCount++;
	}
}

void UQuestService::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UQuestService, QuestPull);
}

void UQuestService::OnRep_QuestPull()
{
	OnQuestPullChanged.Broadcast();
}
