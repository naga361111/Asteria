// Copyright Epic Games, Inc. All Rights Reserved.

#include "BTTask_UseItem.h"

#include "Items/ItemUse.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_UseItem::UBTTask_UseItem()
{
	NodeName = TEXT("Use Item");
}

EBTNodeResult::Type UBTTask_UseItem::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AI = OwnerComp.GetAIOwner();
	APawn* Pawn = AI ? AI->GetPawn() : nullptr;
	if (Pawn == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	// 아이템 id = 지정한 Item 행의 이름(효과 테이블도 같은 이름으로 키잉).
	return UseItem(EffectTable, Item.RowName, Pawn) ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
