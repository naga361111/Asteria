// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CounterEntryObject.generated.h"

/**
 * TileView의 item source 한 칸. 창구 제출함의 ClaimId 하나를 UObject로 승격시킨 view-model.
 *
 * CounterService가 소유한 건 ID뿐이라 여기도 ID만 든다.
 * QuestId·파티 같은 Claim 본체 값을 칸에 그리려면 QuestService에 ClaimId 역참조 접근자가 필요한데
 * 아직 없다. 열리면 여기에 파생 필드를 추가하고 보드가 join해 채운다
 * (UQuestEntryObject의 bClaimed와 같은 방식 — join은 칸이 아니라 컨테이너의 일).
 */
UCLASS()
class ASTERIA_API UCounterEntryObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 ClaimId = INDEX_NONE;
};
