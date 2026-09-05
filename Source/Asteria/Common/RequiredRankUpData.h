// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Rank.h"
#include "Engine/DataAsset.h"
#include "RequiredRankUpData.generated.h"

/**
 * 
 */
UCLASS()
class ASTERIA_API URequiredRankUpData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly)
	TMap<ERank, int32> RequiredClears;
};
