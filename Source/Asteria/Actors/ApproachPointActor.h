// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ApproachPointActor.generated.h"

UCLASS()
class ASTERIA_API AApproachPointActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AApproachPointActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
