// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Engine/DataTable.h" // FDataTableRowHandle
#include "BTTask_UseItem.generated.h"

/** 먹기 전용 태스크 — BT 에셋에 지정된 아이템 하나를 사용해 빙의 폰의 허기를 회복한다.
 *  적용되면 Succeeded, 아니면 Failed. 아이템은 BB가 아니라 이 노드에 수동 할당. */
UCLASS()
class ASTERIA_API UBTTask_UseItem : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_UseItem();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	/** 사용할 아이템(Item 테이블 행). BT 에셋에서 드롭다운으로 지정. */
	UPROPERTY(EditAnywhere, Category="Item", meta=(RowType="/Script/Asteria.ItemDef"))
	FDataTableRowHandle Item;

	/** 아이템 효과 정의 테이블(FItemEffectDef 행). 아이템 id(행 이름)로 효과를 조회. */
	UPROPERTY(EditAnywhere, Category="Item")
	TObjectPtr<UDataTable> EffectTable;
};
