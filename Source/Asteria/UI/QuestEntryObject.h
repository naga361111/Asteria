// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Quest/Quest.h"
#include "UObject/Object.h"
#include "QuestEntryObject.generated.h"

/**
 * 
 */
UCLASS()
class ASTERIA_API UQuestEntryObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FQuest Quest;
};
