// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NPC/NpcActionDef.h" // ENpcNeed
#include "ItemEffectDef.generated.h"

/** 아이템 효과 정의(DataTable 행). Item 테이블의 아이템 하나를 참조해 효과를 붙인다.
 *  Item은 표준 행 핸들 — Item DataTable을 고르면 그 안 행 전체가 드롭다운으로 뜬다(카테고리 필터 없음).
 *  조회는 행 이름으로 한다(행 이름 = 아이템 id) — Item 핸들은 명시적 링크·드롭다운용.
 *  데이터는 사용자가 채운다. */
USTRUCT(BlueprintType)
struct FItemEffectDef : public FTableRowBase
{
	GENERATED_BODY()

	/** 효과를 받을 아이템(Item 테이블의 한 행). RowType로 FItemDef 테이블만 걸리도록 제한. */
	UPROPERTY(EditAnywhere, Category="Effect", meta=(RowType="/Script/Asteria.ItemDef"))
	FDataTableRowHandle Item;

	/** 사용 시 각 고려값에 그대로 더할 변화량(음수 = 감소). 키 없음 = 그 효과 없음.
	 *  Hunger는 높을수록 배고픔이라 음식은 음수(예: Hunger -30). 키 종류는 ENpcNeed에서 옴. */
	UPROPERTY(EditAnywhere, Category="Effect")
	TMap<ENpcNeed, int32> NeedDeltas;
};
