// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FaceDestination.generated.h"

/**
 * Object 키에 담긴 목적지 액터의 yaw로 폰을 회전시킨다.
 * MoveTo 뒤에 두어 "도착 지점에서 목적지 액터가 향한 방향"을 재현하는 용도.
 * 폰은 bOrientRotationToMovement 기준이라 정지 상태의 액터 회전이 곧 권위 → 복제 무브먼트로 클라 동기화.
 */
UCLASS()
class ASTERIA_API UBTTask_FaceDestination : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FaceDestination();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	// 목적지 액터를 담은 Object 키. 이 액터의 yaw를 최종 방향으로 사용한다.
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetKey;

	// 0 이하면 즉시 스냅, 초과면 RInterpTo 속도로 보간 회전한다.
	UPROPERTY(EditAnywhere, Category="Rotation")
	float InterpSpeed = 8.0f;

	// 목표 yaw와의 차이가 이 각도(도) 이내로 들어오면 성공 처리한다.
	UPROPERTY(EditAnywhere, Category="Rotation", meta=(ClampMin="0.0"))
	float AngleTolerance = 1.0f;
};
