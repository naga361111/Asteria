// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_WaitForConfirmQuest.generated.h"

/**
 * 퀘스트 확정을 무한정 대기한다. 외부(서버 권위)에서 FinishLatentTask를 호출하기 전까지 끝나지 않는다.
 */
UCLASS()
class ASTERIA_API UBTTask_WaitForConfirmQuest : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_WaitForConfirmQuest();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	// 노드는 인스턴스 공유 → 실행별 상태(구독 핸들 등)는 NodeMemory에.
	virtual uint16 GetInstanceMemorySize() const override;
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;
};
