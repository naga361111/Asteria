// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UDataTable;
class AActor;

/** 아이템 하나 사용 — EffectTable에서 ItemId 행(FItemEffectDef)을 조회해 User의 NeedsComponent에 적용.
 *  효과 행은 아이템 RowName으로 키잉(행 이름 = 아이템 id). 지금 효과는 허기 회복 하나.
 *  효과 행 없음·User에 NeedsComponent 없음이면 아무것도 안 하고 false. */
bool UseItem(const UDataTable* EffectTable, FName ItemId, AActor* User);
