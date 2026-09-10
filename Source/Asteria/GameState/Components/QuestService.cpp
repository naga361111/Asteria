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
	DOREPLIFETIME(UQuestService, Assignments);
}

void UQuestService::OnRep_QuestPull()
{
	OnQuestPullChanged.Broadcast();
}

void UQuestService::OnRep_Assignments()
{
	OnAssignmentsChanged.Broadcast();
}

int32 UQuestService::AssignQuest(int32 QuestId, const TArray<int32>& Party)
{
	// 소유·복제 방향 불변조건: 상태 변경은 호스트만.
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return INDEX_NONE;
	}

	// 입력 검증(경계): 존재하는 퀘스트인가 / 파티가 비지 않았는가.
	// TODO: 이미 배정된 퀘스트가 아닌지 / 파티 NPC가 다른 배정에 묶여있지 않은지도 여기서 검증.
	const bool bQuestExists = QuestPull.ContainsByPredicate(
		[QuestId](const FQuest& Q) { return Q.QuestId == QuestId; });
	if (!bQuestExists || Party.Num() == 0)
	{
		return INDEX_NONE;
	}

	FQuestAssignment Assignment;
	Assignment.AssignmentId = AssignmentCount++;
	Assignment.QuestId = QuestId;
	Assignment.Party = Party;

	Assignments.Add(Assignment);

	// 서버는 OnRep이 자동 호출되지 않으므로 직접 통지.
	OnAssignmentsChanged.Broadcast();

	return Assignment.AssignmentId;
}
