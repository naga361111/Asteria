// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemDef.generated.h"

/** 아이템 카테고리. 지금은 음식 하나. 늘어나고 각자 다른 데이터가 필요해지면 그때 분리 판단. */
UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	Food,
};

/** 아이템 하나의 정의(DataTable 행). 이름과 카테고리만 — 효과는 별도 Effect 테이블에서 부착.
 *  Id는 행의 RowName이 곧 Id(별도 필드 없음). 데이터는 사용자가 채운다. */
USTRUCT(BlueprintType)
struct FItemDef : public FTableRowBase
{
	GENERATED_BODY()

	/** 표시 이름. "빵" 등. */
	UPROPERTY(EditAnywhere, Category="Item")
	FText DisplayName;

	/** 카테고리. 드롭다운은 EItemCategory에서 옴. */
	UPROPERTY(EditAnywhere, Category="Item")
	EItemCategory Category = EItemCategory::Food;
};
