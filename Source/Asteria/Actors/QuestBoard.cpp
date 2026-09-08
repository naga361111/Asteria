// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/QuestBoard.h"
#include "GameFramework/PlayerController.h"

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
	APlayerController* PC = Interactor->GetController<APlayerController>();
	if (!PC->bShowMouseCursor)
	{
		PC->bShowMouseCursor = true;
		PC->SetInputMode(FInputModeGameAndUI());
	}
	else
	{
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}
}

