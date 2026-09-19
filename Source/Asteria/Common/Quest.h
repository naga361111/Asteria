#pragma once

#include "CoreMinimal.h"
#include "Rank.h"
#include "Quest.generated.h"

// 퀘스트 정의. "무엇"만 담는다 — 진행 상태(누가·어디까지)는 FQuestAssignment가 소유한다.
USTRUCT()
struct FQuest
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	int32 QuestId = INDEX_NONE;
	
	UPROPERTY(VisibleAnywhere)
	ERank QuestRnk = ERank::F;
};

// Assignment의 생애 단계. 값을 가르는 축은 "다음에 움직여야 할 주체"다.
//
// 해제(반려·포기·NPC 소멸)는 상태 값이 아니라 QuestAssignments 배열에서의 제거로 표현한다.
// 그래서 "중단"에는 terminal 값이 없고, 퀘스트 가용성이 "살아있는 Assignment의 존재"로 계속 파생된다.
// 완수 이후(Cleared·Settled)만은 예외로 값으로 남긴다 — 보상·평판 정산의 근거가 제거와 함께 사라지면 안 되기 때문.
UENUM()
enum class EQuestAssignmentState : uint8
{
	// 보드에서 집었다. 카운터로 이동 중 — 다음 행위자는 NPC.
	Assigned,
	// 카운터 제출함에 올라갔다 — 다음 행위자는 플레이어.
	Submitted,
	// 플레이어가 수주를 확정했다 — 다음 행위자는 NPC(던전 수행).
	Accepted,
	// 수행이 끝났다. 창구 정산이 남았다 — 다음 행위자는 플레이어.
	Cleared,
	// 카운터의 보상 대기 제출함에 올라갔다 - 다음 행위자는 플레이어.
	SubmitForSettled,
	// 창구에서 보상·평판까지 정산됐다. 다음 행위자가 없는 유일한 terminal 값 — 여기서 더 전이하지 않는다.
	Settled,
};

// 퀘스트-NPC 소유권의 단일 진실원. Quest도 NPC도 서로를 가리키지 않고,
// 둘 다 안정 id로 여기서 참조되기만 한다. QuestService가 소유·복제.
//
// "어디까지 진행됐나"도 State 하나가 전부다. 제출 여부 같은 단계를 별도 목록으로
// 중복 표현하지 않는다 — 진실원이 둘이면 반드시 어긋난다(제출함엔 있는데 이미 Accepted 등).
USTRUCT()
struct FQuestAssignment
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	int32 AssignmentId = INDEX_NONE;

	UPROPERTY(VisibleAnywhere)
	int32 QuestId = INDEX_NONE;

	// 집은 NPC들의 안정 id (ANpc* 아님 — 수명/복제 안전). 1인이면 원소 1개.
	UPROPERTY(VisibleAnywhere)
	TArray<int32> Party;

	// 쓰기는 QuestService의 전이 함수만(서버 권위). 읽기는 복제된 값으로 클라도 동일 판정.
	UPROPERTY(VisibleAnywhere)
	EQuestAssignmentState State = EQuestAssignmentState::Assigned;
};
