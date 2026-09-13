// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CounterService.generated.h"

class UQuestService;

/**
 * 창구(카운터)의 경계. 상태는 하나도 소유하지 않는다.
 *
 * "제출됨"은 Assignment의 State(EQuestAssignmentState::Submitted)가 전부다. 창구가 제출 목록을
 * 따로 복제해 들면 같은 사실의 진실원이 둘이 되고, Accept 시 한쪽만 갱신되는 순간 어긋난다.
 * 그래서 여기는 읽기 판정을 파생시키고, 쓰기는 소유자(QuestService)에 위임만 한다.
 *
 * 의존 방향: CounterService → QuestService. 창구가 Assignment를 검사해 통과시키는 쪽이지
 * 그 반대가 아니다. 반려·수락 입력이 붙을 자리도 여기다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ASTERIA_API UCounterService : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCounterService();

	// NPC가 컨펌 요구 퀘스트를 창구에 올린다(Assigned→Submitted).
	// 권위·실재·상태 검증은 전부 QuestService가 한다 — 여기서 또 하면 검증이 두 벌이 된다.
	bool SubmitQuestAssignment(int32 AssignmentId);

	// NPC가 완수한 퀘스트를 창구의 보상 대기 제출함에 올린다(Cleared→SubmitForSettled).
	// 수주 제출과 같은 문(창구)을 쓴다 — 검증은 QuestService가 한 벌로 한다.
	bool SubmitForSettleQuestAssignment(int32 AssignmentId);

	// 플레이어가 창구에서 수주를 확정한다(Submitted→Accepted).
	// 창구 UI의 수락 버튼이 도달하는 종착점. 검증은 여기서 하지 않는다 — 위와 같은 이유.
	bool AcceptQuestAssignment(int32 AssignmentId);

	// 제출 여부는 Assignment의 State에서 파생한다.
	bool IsQuestAssignmentSubmitted(int32 AssignmentId) const;

private:
	// 두 서비스 모두 GameState의 컴포넌트라 오너를 거쳐 찾는다.
	UQuestService* GetQuestService() const;
};
