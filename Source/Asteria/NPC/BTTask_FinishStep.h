// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FinishStep.generated.h"

/** 한 원자 BT의 끝에 붙는 태스크. 실행되면 컨트롤러의 ActionRunner를 찾아
 *  Advance()를 불러 "이 원자 끝났다 → 다음으로" 신호를 준다. */
UCLASS()
class ASTERIA_API UBTTask_FinishStep : public UBTTaskNode
{
	GENERATED_BODY()

public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
