// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Despawn.generated.h"

/**
 * 트리를 돌리는 NPC 폰을 서버에서 지연 제거(SetLifeSpan) 예약하고, 제거될 때까지 InProgress로 대기한다.
 * 폰이 제거되면 엔진이 AI 컨트롤러와 BT를 함께 정리한다.
 */
UCLASS()
class ASTERIA_API UBTTask_Despawn : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Despawn();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
