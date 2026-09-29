// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Navigation/PathFollowingComponent.h"
#include "Actors/ApproachPointActor.h"
#include "BTTask_WaitForSettleConfirm.generated.h"

class UQuestService;
class UGameClockService;

/**
 * 보상 대기 제출함에 올라간(SubmitForSettled) 퀘스트의 정산 컨펌을 대기 지점(기본 Lounge)으로 걸어가며 기다린다.
 * 이동 중 컨펌되면 즉시 Succeeded, 도착(또는 이동 실패) 후에는 제자리에서 컨펌까지 대기.
 * 제출 시점부터 WaitGameMinutes(게임 분)가 지나도 컨펌되지 않으면 Failed.
 * 제출은 앞선 Submit Settle이, 수령·지급은 뒤따르는 Receive Settle이 맡는다.
 * 이동·회전은 MoveToApproachPoint와 같은 방식으로 자체 구현한다.
 */
UCLASS()
class ASTERIA_API UBTTask_WaitForSettleConfirm : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_WaitForSettleConfirm();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

protected:
	// 이동 완료 콜백. 도착하면 회전 페이즈로 전환한다. 어느 경우에도 태스크는 끝내지 않는다.
	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

	// 대기할 접근 지점의 종류.
	UPROPERTY(EditAnywhere, Category = "Approach")
	EApproachPointType TargetType = EApproachPointType::Lounge;

	// 이동 도착 허용 반경.
	UPROPERTY(EditAnywhere, Category = "Move", meta = (ClampMin = "0.0"))
	float AcceptableRadius = 50.0f;

	// 0 이하면 즉시 스냅, 초과면 RInterpTo 속도로 보간 회전한다.
	UPROPERTY(EditAnywhere, Category = "Rotation")
	float InterpSpeed = 8.0f;

	// 목표 yaw와의 차이가 이 각도(도) 이내면 회전 완료 처리.
	UPROPERTY(EditAnywhere, Category = "Rotation", meta = (ClampMin = "0.0"))
	float AngleTolerance = 1.0f;

	// 정산 컨펌 대기 제한 시간(게임 분). 제출 시점부터 세며, 넘기면 Failed.
	UPROPERTY(EditAnywhere, Category = "Wait", meta = (ClampMin = "1"))
	int32 WaitGameMinutes = 30;

private:
	// 이동 완료 델리게이트를 떼고 이동 요청 ID를 초기화한다.
	void UnbindMoveCompleted();

	// 목표 지점 yaw로 한 틱 회전. 다 돌았으면 true.
	bool StepFacing(APawn& Pawn, float DeltaSeconds) const;

	// 이번 실행에서 찾은 대기 지점.
	UPROPERTY()
	TObjectPtr<AApproachPointActor> TargetPoint = nullptr;

	// 이동 완료 콜백·델리게이트 해제에 필요한 소유 컴포넌트.
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

	// 진행 중인 이동 요청 ID(스테일 콜백 무시용).
	FAIRequestID MoveRequestID;

	// 이동을 끝내고 회전 페이즈에 들어갔는지.
	bool bFacing = false;

	// 구독 해제용 서비스.
	TWeakObjectPtr<UQuestService> QuestService;

	// 정산 컨펌 알림 구독 핸들.
	FDelegateHandle SettledHandle;

	// 기다리는 Assignment id.
	int32 WaitingAssignmentId = INDEX_NONE;

	// 마감 확인용 게임 시계.
	TWeakObjectPtr<UGameClockService> GameClock;

	// 이번 실행의 대기 마감 게임 분.
	int32 ExpireGameMinute = 0;
};
