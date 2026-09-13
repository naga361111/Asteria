// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_DoQuest.generated.h"

/**
 * 수주 확정된(Accepted) 자기 퀘스트를 수행하고 완료로 확정한다(Accepted→Cleared).
 *
 * 수행 내용 자체는 아직 없다 — QuestDuration만큼 대기하는 자리표시자다.
 * 던전 코어가 서면 이 대기만 교체되고, 앞뒤(무엇을 수행하나·완료를 누가 기록하나)는 그대로 남는다.
 */
UCLASS()
class ASTERIA_API UBTTask_DoQuest : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_DoQuest();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// 노드는 인스턴스 공유 → 실행별 상태(대상 Assignment·남은 시간)는 NodeMemory에.
	virtual uint16 GetInstanceMemorySize() const override;
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;

	// 수행에 걸리는 시간(초). 던전 코어가 설 때까지의 자리표시자.
	UPROPERTY(EditAnywhere, Category="Quest", meta=(ClampMin="0.0"))
	float QuestDuration = 5.f;
};
