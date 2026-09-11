// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Common/Quest.h"
#include "QuestEntryObject.generated.h"

/**
 * TileView의 item source 한 칸. FQuest 하나를 UObject로 승격시킨 view-model.
 */
UCLASS()
class ASTERIA_API UQuestEntryObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FQuest Quest;

	// 파생 상태: QuestService->IsQuestClaimed()로 보드가 join해 채운다.
	UPROPERTY()
	bool bClaimed = false;
};
