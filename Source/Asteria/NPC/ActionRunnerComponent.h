// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActionRunnerComponent.generated.h"

class UNpcActionSet;

/** 브레인이 고른 행동(NpcActionSet)을 실행하는 재생기. AIController에 붙는다.
 *  지금은 Set에 원자 BT 1개 가정 — 그 하나를 RunBehaviorTree로 돌린다.
 *  (여러 개 큐/전진/정지는 나중.) */
UCLASS(ClassGroup=(NPC), meta=(BlueprintSpawnableComponent))
class ASTERIA_API UActionRunnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UActionRunnerComponent();

	/** Set의 첫 원자 BT를 실행하고 목적지를 blackboard에 쓴다.
	 *  Set이 null·비었으면 StopLogic으로 정지(Idle). */
	void RunSet(UNpcActionSet* Set, const FVector& Target);

	/** 이동 BT가 읽을 blackboard 키. 러너가 목적지를 이 키에 쓴다. */
	UPROPERTY(EditAnywhere, Category="Action Runner")
	FName MoveGoalKey = "MoveGoal";
};
