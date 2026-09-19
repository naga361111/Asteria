// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Navigation/PathFollowingComponent.h"
#include "Actors/ApproachPointActor.h"
#include "BTTask_MoveToApproachPoint.generated.h"

class AApproachPointActor;

/**
 * FindApproachPoint → MoveTo → FaceDestination 3노드 세트를 하나로 합친 태스크.
 * (1) NPC 기준 지정한 종류의 가장 가까운 ApproachPointActor를 찾고,
 * (2) 그 지점으로 이동한 뒤,
 * (3) 그 지점의 yaw로 폰을 회전시킨다.
 * 서버(호스트) AIController에서만 실행 → pathfollowing·CharacterMovement가 복제를 담당한다.
 * 실행 중 상태(대상·이동 요청·페이즈)를 멤버에 저장하므로 bCreateNodeInstance로 인스턴스화한다.
 */
UCLASS()
class ASTERIA_API UBTTask_MoveToApproachPoint : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MoveToApproachPoint();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	// 이동 완료 콜백. 도착하면 회전 페이즈로 전환한다.
	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

	// 찾을 접근 지점의 종류.
	UPROPERTY(EditAnywhere, Category = "Approach")
	EApproachPointType TargetType = EApproachPointType::Counter;

	// 이동 도착 허용 반경.
	UPROPERTY(EditAnywhere, Category = "Move", meta = (ClampMin = "0.0"))
	float AcceptableRadius = 50.0f;

	// 0 이하면 즉시 스냅, 초과면 RInterpTo 속도로 보간 회전한다.
	UPROPERTY(EditAnywhere, Category = "Rotation")
	float InterpSpeed = 8.0f;

	// 목표 yaw와의 차이가 이 각도(도) 이내면 회전 성공 처리.
	UPROPERTY(EditAnywhere, Category = "Rotation", meta = (ClampMin = "0.0"))
	float AngleTolerance = 1.0f;

private:
	// 이동 완료 델리게이트를 떼고 대기를 정리한다(성공·중단 공통).
	void UnbindMoveCompleted();

	// 목표 지점 yaw로 회전 완료 여부를 갱신. 다 돌았으면 true.
	bool StepFacing(APawn& Pawn, float DeltaSeconds) const;

	// 이번 실행에서 찾은 접근 지점.
	UPROPERTY()
	TObjectPtr<AApproachPointActor> TargetPoint = nullptr;

	// FinishLatentTask 호출에 필요한 소유 컴포넌트.
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

	// 진행 중인 이동 요청 ID(스테일 콜백 무시용).
	FAIRequestID MoveRequestID;

	// 이동을 끝내고 회전 페이즈에 들어갔는지.
	bool bFacing = false;
};
