// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ApproachPointActor.generated.h"

UENUM(BlueprintType)
enum class EApproachPointType : uint8
{
	Counter,
	QuestBoard,
	Dungeon,
};

UCLASS()
class ASTERIA_API AApproachPointActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AApproachPointActor();

	// 이 접근 지점의 역할. NPC가 종류별로 가장 가까운 지점을 찾을 때의 쿼리 키.
	UPROPERTY(EditAnywhere, Category = "Approach")
	EApproachPointType PointType = EApproachPointType::Counter;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
