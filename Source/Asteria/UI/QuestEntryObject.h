// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QuestEntryObject.generated.h"

struct FQuest;
/**
 * 
 */
UCLASS()
class ASTERIA_API UQuestEntryObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 QuestId;
};
