// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NpcActionSet.h"
#include "ActionRunnerComponent.generated.h"

class UNpcActionSet;
class UBehaviorTree;
class UPlaceComponent;

/** 브레인이 고른 행동(NpcActionSet)을 실행하는 재생기. AIController에 붙는다.
 *  Set의 원자 BT들을 순서대로 하나씩 실행: BT 시작 → 그 BT 끝의 FinishStep이
 *  Advance() 호출 → 다음 BT. 소진되면 StopLogic으로 정지(Idle). */
UCLASS(ClassGroup=(NPC), meta=(BlueprintSpawnableComponent))
class ASTERIA_API UActionRunnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UActionRunnerComponent();

	/** Set의 원자 BT들을 작업 큐로 복사하고 첫 BT부터 실행. Set이 null·비면 즉시 정지. */
	void RunSet(UNpcActionSet* Set, const FVector& InTarget);

	/** 현재 원자 BT가 끝났다는 신호(FinishStep이 호출). 다음 원자로 전진. */
	void Advance();

	/** 이동 BT가 읽을 blackboard 키. 러너가 목적지를 이 키에 쓴다. */
	UPROPERTY(EditAnywhere, Category="Action Runner")
	FName MoveGoalKey = "MoveGoal";

	/** 디버그용 관측: 지금 실행 중인 원자 BT(없으면 null), 진행 인덱스/개수. */
	UBehaviorTree* GetCurrentBehavior() const { return Sequence.IsValidIndex(Index) ? Sequence[Index].BT : nullptr; }
	int32 GetStepIndex() const { return Index; }
	int32 GetStepCount() const { return Sequence.Num(); }

private:
	/** Sequence[Index] 원자 BT를 실행하고 목적지를 blackboard에 쓴다. 범위 밖이면 Stop. */
	void RunCurrent();

	/** 큐 비우고 StopLogic으로 트리 정지(완전 Idle). */
	void Stop();

	/** Category가 일치하고 RequiredPoints의 지점 종류를 모두 가진 건물 중 From에 최근접한 것.
	 *  없으면 null. 행동 전환 때만 부르는 전수 스캔(ponytail: 느려지면 레지스트리). */
	static UPlaceComponent* FindNearestPlace(const UWorld* World, const FVector& From,
		EBuildingType Category, const TSet<EPlacePoint>& RequiredPoints);

	/** 실행할 스텝 배열(세트 복사본). 순서대로 소비된다. */
	UPROPERTY(VisibleInstanceOnly, Transient, Category="Action Runner")
	TArray<FActionStep> Sequence;

	/** 지금 실행 중인 원자의 인덱스. */
	int32 Index = 0;

	/** 이 세트의 목적지. 원자마다 blackboard에 다시 쓴다. */
	FVector Target = FVector::ZeroVector;

	/** 이 행동이 이동할 건물. 최초 이동 스텝에서 확정하고, 남은 이동 스텝이 재사용한다. */
	UPROPERTY(Transient)
	TObjectPtr<UPlaceComponent> TargetPlace = nullptr;
};
