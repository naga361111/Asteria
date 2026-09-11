#pragma once

#include "CoreMinimal.h"
#include "Quest.generated.h"

// 퀘스트 정의. "무엇"만 담는다 — 진행 상태(누가·어디까지)는 FQuestClaim이 소유한다.
USTRUCT()
struct FQuest
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	int32 QuestId = INDEX_NONE;
};

// 퀘스트-NPC 소유권의 단일 진실원. Quest도 NPC도 서로를 가리키지 않고,
// 둘 다 안정 id로 여기서 참조되기만 한다. QuestService가 소유·복제.
//
// Claim의 "존재" 자체가 곧 "보드에서 집혀 컨펌 대기 중(pending)" 상태다.
// 카운터에서 플레이어가 최종 수주를 확정하는 단계를 구현할 때,
// 여기에 EQuestClaimState{Pending, Confirmed} 필드를 추가하고
// QuestService에 ConfirmClaim(Pending→Confirmed) 전이를 연다.
USTRUCT()
struct FQuestClaim
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	int32 ClaimId = INDEX_NONE;

	UPROPERTY(VisibleAnywhere)
	int32 QuestId = INDEX_NONE;

	// 집은 NPC들의 안정 id (ANpc* 아님 — 수명/복제 안전). 1인이면 원소 1개.
	UPROPERTY(VisibleAnywhere)
	TArray<int32> Party;
};
