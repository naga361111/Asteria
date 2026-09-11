// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestBoard.h"

#include "GameState/AsteriaGameState.h"
#include "GameState/Components/CounterService.h"


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

bool AQuestBoard::CanInteract() const
{
	return true;
}

void AQuestBoard::OnInteract(AAsteriaPlayer* Interactor)
{
	AAsteriaGameState* GS = GetWorld()->GetGameState<AAsteriaGameState>();
	if (GS == nullptr) return;

	UQuestService* Service = GS->QuestService;
	if (Service == nullptr) return;

	UCounterService* Counter = GS->CounterService;
	if (Counter == nullptr) return;
	
	UE_LOG(LogTemp, Warning, TEXT("Overlapped: %d"), Counter->SubmittedClaimIds.Num())
}
