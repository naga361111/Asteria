// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Common/Quest.h"
#include "QuestService.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnQuestPullChanged);
// QuestAssignments가 바뀌었음을 알리는 신호. 원소 추가·제거뿐 아니라 State 전이도 여기로 온다 —
// 제출함처럼 QuestAssignments에서 파생되는 모든 뷰가 이 하나를 구독한다.
DECLARE_MULTICAST_DELEGATE(FOnQuestAssignmentsChanged);
// 특정 Assignment가 Submitted→Accepted로 확정됐음을 알리는 서버 로컬 신호. 인자는 확정된 AssignmentId.
// (WaitForConfirm BT 태스크가 자기 AssignmentId만 필터해 FinishLatentTask 호출용. 복제 아님)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnQuestAssignmentAccepted, int32 /*AssignmentId*/);
// 특정 Assignment가 SubmitForSettled→Settled로 정산 확정됐음을 알리는 서버 로컬 신호. 인자는 정산된 AssignmentId.
// (WaitForSettleConfirm BT 태스크가 자기 AssignmentId만 필터해 FinishLatentTask 호출용. 복제 아님)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnQuestAssignmentSettled, int32 /*AssignmentId*/);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ASTERIA_API UQuestService : public UActorComponent
{
	GENERATED_BODY()

	int32 QuestCount = 0;
	int32 AssignmentCount = 0;

public:
	// Sets default values for this component's properties
	UQuestService();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_QuestPull, Category="Quest")
	TArray<FQuest> QuestPull;

	UFUNCTION()
	void OnRep_QuestPull();

	// QuestPull이 바뀌면 이걸 Broadcast (호출자 책임)
	FOnQuestPullChanged OnQuestPullChanged;

	// --- 소유권(Assignment): 퀘스트-NPC 소유·진행의 단일 진실원 ---

	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_QuestAssignments, Category="Quest")
	TArray<FQuestAssignment> QuestAssignments;

	UFUNCTION()
	void OnRep_QuestAssignments();

	FOnQuestAssignmentsChanged OnQuestAssignmentsChanged;
	
	FOnQuestAssignmentAccepted OnQuestAssignmentAccepted;

	FOnQuestAssignmentSettled OnQuestAssignmentSettled;

	// --- 정산 대기: 창구 정산을 기다리는 Assignment들 ---

	// 쓰기는 서버 권위, 클라는 복제된 값을 읽기만 한다.
	// QuestAssignments의 사본이므로 원본이 전이·제거될 때 함께 갱신하는 건 넣는 쪽 책임이다.
	UPROPERTY(VisibleAnywhere, Replicated, Category="Quest")
	TArray<FQuestAssignment> SettleQuestAssignments;

	// 정산 대기열에 올린다(Cleared인 것만). 실패(권위 없음/미존재/상태 불일치/이미 대기 중) 시 false.
	// 상태 전이가 아니라 대기열 등재다 — Cleared→Settled 전이는 창구가 정산할 때 따로 일어난다.
	bool EnqueueSettleQuestAssignment(int32 AssignmentId);

	// 정산 대기열에서 내린다. 정산 완료·대기 중단(Abort·NPC 소멸) 어느 쪽이든 이 하나로 회수한다.
	// 올린 쪽이 내리는 것까지 책임진다 — 안 그러면 원본이 사라져도 사본이 남는다.
	bool DequeueSettleQuestAssignment(int32 AssignmentId);

	// --- 상태 전이: 전부 서버 권위. QuestAssignments를 쓰는 곳은 여기뿐이다. ---

	// ∅→Assigned. 퀘스트를 파티가 집는다(보드 선택 = 소유권 획득). Select+Assign을 하나로.
	// 성공 시 AssignmentId, 실패(권위 없음/미존재/이미 집힘/빈 파티) 시 INDEX_NONE.
	int32 AssignQuest(int32 QuestId, const TArray<int32>& Party);

	// Assigned→Submitted. NPC가 카운터에 도착해 컨펌을 요구한다.
	// 제출됨은 이 상태값이 전부다 — 창구가 별도 목록을 들지 않는다.
	bool SubmitQuestAssignment(int32 AssignmentId);

	// Submitted→Accepted. 플레이어가 창구에서 수주를 확정한다.
	// 입력 경로는 카운터 UI 버튼 → 플레이어(소유 액터)의 Server RPC → CounterService → 여기.
	// 클라가 보낸 AssignmentId는 그 사이 사라졌거나 상태가 어긋났을 수 있으므로 여기서 다시 검증한다.
	bool AcceptQuestAssignment(int32 AssignmentId);

	// Accepted→Cleared. NPC가 수행을 마친다. 실행 경로는 던전 수행(BT) → 여기.
	// 다음은 창구 정산(Cleared→Settled)이고, 중복 완료는 From 검사에서 막힌다.
	bool ClearQuestAssignment(int32 AssignmentId);

	// Cleared→SubmitForSettled. NPC가 완수한 퀘스트를 창구의 보상 대기 제출함에 올린다.
	// 올라가 있음은 이 상태값이 전부다 — 창구가 별도 목록을 들지 않는다(SubmitQuestAssignment와 같은 이유).
	bool SubmitForSettleQuestAssignment(int32 AssignmentId);

	// SubmitForSettled→Settled. 플레이어가 창구에서 정산을 확정한다. 유일한 terminal 전이다.
	// 입력 경로는 정산 UI 버튼 → 플레이어(소유 액터)의 Server RPC → CounterService → 여기.
	// AcceptQuestAssignment와 같은 이유로 클라가 보낸 AssignmentId를 여기서 다시 검증한다.
	bool SettleQuestAssignment(int32 AssignmentId);

	// --- 파생 질의: 상태에서 읽어낼 뿐 따로 저장하지 않는다 ---

	// 가용성 파생의 단일 원천. 클라에서도 복제된 QuestAssignments를 읽어 동일 판정.
	// 살아있는 Assignment가 하나라도 있으면 집힌 것 — 해제는 원소 제거로 표현되므로 상태는 안 본다.
	bool IsQuestAssigned(int32 QuestId) const;

	// 아직 안 집힌 퀘스트의 QuestId 하나. 없으면 INDEX_NONE. (BT의 "무엇을 집을까" 선택용)
	int32 FindAvailableQuestId() const;

	// AssignmentId → Assignment 본체 역참조. 없으면 nullptr.
	// 반환 포인터는 QuestAssignments가 바뀌면 무효다(재할당·복제 갱신) — 즉시 읽고 버릴 것.
	const FQuestAssignment* FindQuestAssignment(int32 AssignmentId) const;

	// NpcId → 그 NPC가 파티원인 Assignment. 없으면 nullptr. (BT가 "내 퀘스트가 뭐였지"를 묻는 경로)
	// State를 반드시 받는 이유: 한 NPC가 단계가 다른 Assignment를 동시에 들 수 있어
	// 단계를 안 좁히면 엉뚱한 걸 집는다. 반환 포인터 수명 주의는 위와 같다.
	const FQuestAssignment* FindQuestAssignmentByNpc(int32 NpcId, EQuestAssignmentState State) const;

private:
	// 모든 전이의 공통 경로: 권위 확인 → 실재·현재 상태 확인 → 쓰기 → 변경 통지.
	// 전이마다 다른 건 From/To뿐이라 불변조건 검사를 한 벌로 유지한다.
	// 성공 시 갱신된 원소, 실패 시 nullptr. 반환 포인터는 즉시 쓰고 버릴 것.
	FQuestAssignment* TransitionQuestAssignment(int32 AssignmentId, EQuestAssignmentState From, EQuestAssignmentState To);
};
