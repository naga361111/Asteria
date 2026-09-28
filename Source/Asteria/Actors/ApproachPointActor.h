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
	Disappear,
	Lounge,
	// NPC가 나타나는 지점. NpcSpawnService가 여럿 중 무작위로 고른다.
	Spawn,
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

	// 이 지점을 예약한 폰. 서버 전용이라 복제·UPROPERTY 없음 — 폰이 사라지면 약참조가 풀려 자동으로 빈 자리.
	TWeakObjectPtr<APawn> Occupant;

	// Origin 기준 해당 종류이면서 예약되지 않은 가장 가까운 지점(2D 거리). 없으면 null.
	static AApproachPointActor* FindNearestFree(UWorld* World, EApproachPointType Type, const FVector& Origin);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
