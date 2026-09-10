#pragma once

#include "CoreMinimal.h"
#include "Quest.generated.h"

UENUM()
enum class EQuest: uint8
{
	Generated,
	Selected,
	Accepted,
	Cleared
};

USTRUCT()
struct FQuest
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	int32 QuestId;

	UPROPERTY(VisibleAnywhere)
	EQuest QuestType;
};

// 퀘스트-NPC 소유권의 단일 진실원. Quest도 NPC도 서로를 가리키지 않고,
// 둘 다 안정 id로 여기서 참조되기만 한다. QuestService가 소유·복제.
USTRUCT()
struct FQuestAssignment
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	int32 AssignmentId = INDEX_NONE;

	UPROPERTY(VisibleAnywhere)
	int32 QuestId = INDEX_NONE;

	// 배정된 NPC들의 안정 id (ANpc* 아님 — 수명/복제 안전). 1인이면 원소 1개.
	UPROPERTY(VisibleAnywhere)
	TArray<int32> Party;
};
