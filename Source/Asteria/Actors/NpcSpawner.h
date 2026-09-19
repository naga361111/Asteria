// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NpcSpawner.generated.h"

class AAsteriaNpc;

UCLASS()
class ASTERIA_API ANpcSpawner : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANpcSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, Category = "NPC")
	TSubclassOf<AAsteriaNpc> NpcClass;
};
