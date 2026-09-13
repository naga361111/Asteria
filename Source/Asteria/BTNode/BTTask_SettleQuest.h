// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SettleQuest.generated.h"

/**
 * 완료한(Cleared) 자기 퀘스트를 창구 정산 대기열에 올리고, 정산될 때까지 무한정 대기한다.
 *
 * 여기서 State를 Settled로 옮기지 않는다 — 정산의 행위자는 플레이어이고, 이 태스크는 줄을 서서 기다리는 쪽이다.
 * 대기를 깨우는 경로(창구의 정산 확정 → FinishLatentTask)는 아직 없다. 지금은 등재·대기까지만.
 */
UCLASS()
class ASTERIA_API UBTTask_SettleQuest : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SettleQuest();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	// 노드는 인스턴스 공유 → 실행별 상태(대기 중인 Assignment 등)는 NodeMemory에.
	virtual uint16 GetInstanceMemorySize() const override;
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;
};
