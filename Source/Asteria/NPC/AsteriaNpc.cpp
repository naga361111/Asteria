// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/AsteriaNpc.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AAsteriaNpc::AAsteriaNpc()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AAsteriaNpc::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAsteriaNpc, NpcLevel);
	DOREPLIFETIME(AAsteriaNpc, NpcRank)
}

void AAsteriaNpc::OnRep_Level()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, FString::Printf(TEXT("Lvl: %d"), NpcLevel));
}

void AAsteriaNpc::OnRep_Rank()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, FString::Printf(TEXT("Rnk: %d"), NpcRank));
}

// Called when the game starts or when spawned
void AAsteriaNpc::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AAsteriaNpc::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AAsteriaNpc::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AAsteriaNpc::QuestCleared()
{
	NpcLevelUp();
	NpcRankUp();
}

void AAsteriaNpc::NpcLevelUp()
{
	NpcLevel++;
}

void AAsteriaNpc::NpcRankUp()
{
	if (static_cast<uint8>(NpcRank) >= static_cast<uint8>(ERank::Count) - 1)
		return;
	
	NpcRank = static_cast<ERank>(static_cast<uint8>(NpcRank) + 1);
}
