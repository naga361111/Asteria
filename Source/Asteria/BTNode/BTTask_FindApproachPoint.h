// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Actors/ApproachPointActor.h"
#include "BTTask_FindApproachPoint.generated.h"

/**
 * NPC 기준으로 지정한 종류(EApproachPointType)의 가장 가까운 ApproachPointActor를 찾아
 * 결과 블랙보드 키에 기록한다. 이어지는 MoveTo가 같은 키를 읽어 이동한다.
 */
UCLASS()
class ASTERIA_API UBTTask_FindApproachPoint : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindApproachPoint();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 찾을 접근 지점의 종류.
	UPROPERTY(EditAnywhere, Category = "Approach")
	EApproachPointType TargetType = EApproachPointType::Counter;

	// 찾은 액터를 기록할 블랙보드 키(Object).
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ResultKey;
};
