// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/QuestBoard.h"

// Sets default values
AQuestBoard::AQuestBoard()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AQuestBoard::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AQuestBoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AQuestBoard::AddQuestsPull(const FQuest& Quest)
{
	QuestsPull.Add(Quest);
}

