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
};
