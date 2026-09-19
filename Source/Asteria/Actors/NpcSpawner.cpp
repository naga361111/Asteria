// Fill out your copyright notice in the Description page of Project Settings.


#include "NpcSpawner.h"

#include "NPC/AsteriaNpc.h"


// Sets default values
ANpcSpawner::ANpcSpawner()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// NpcSpawner.cpp 생성자
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

// Called when the game starts or when spawned
void ANpcSpawner::BeginPlay()
{
	Super::BeginPlay();

	// 서버 권위: 스폰은 호스트에서만. 클라는 리플리케이션으로 받는다.
	if (!HasAuthority() || !NpcClass)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AAsteriaNpc* Npc = GetWorld()->SpawnActor<AAsteriaNpc>(
		NpcClass, GetActorLocation(), GetActorRotation(), Params);

	if (Npc)
	{
		// 런타임 스폰은 auto-possess가 안 걸리는 경우가 있어 명시적으로 AI 컨트롤러 부여
		Npc->SpawnDefaultController();
	}
}

// Called every frame
void ANpcSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
