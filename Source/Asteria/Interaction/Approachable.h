// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Approachable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UApproachable : public UInterface
{
	GENERATED_BODY()
};

class ASTERIA_API IApproachable
{
	GENERATED_BODY()
	
public:
	virtual FTransform GetDestinationTransform() const = 0;
};
