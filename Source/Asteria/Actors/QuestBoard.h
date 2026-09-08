// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interactable.h"
#include "Player/AsteriaPlayer.h"
#include "QuestBoard.generated.h"

UCLASS()
class ASTERIA_API AQuestBoard : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AQuestBoard();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	virtual bool CanInteract() const override;
	virtual void OnInteract(AAsteriaPlayer* Interactor) override;

};
