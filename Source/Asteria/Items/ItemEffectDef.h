// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemEffectDef.generated.h"

/** 아이템 효과 정의(DataTable 행). Item 테이블의 아이템 하나를 참조해 효과를 붙인다.
 *  Item은 표준 행 핸들 — Item DataTable을 고르면 그 안 행 전체가 드롭다운으로 뜬다(카테고리 필터 없음).
 *  지금 효과는 허기 회복 하나. 데이터는 사용자가 채운다. */
USTRUCT(BlueprintType)
struct FItemEffectDef : public FTableRowBase
{
	GENERATED_BODY()

	/** 효과를 받을 아이템(Item 테이블의 한 행). RowType로 FItemDef 테이블만 걸리도록 제한. */
	UPROPERTY(EditAnywhere, Category="Effect", meta=(RowType="/Script/Asteria.ItemDef"))
	FDataTableRowHandle Item;

	/** 먹으면 허기에서 빼는 값. */
	UPROPERTY(EditAnywhere, Category="Effect", meta=(ClampMin="0"))
	int32 HungerRestore = 0;
};
