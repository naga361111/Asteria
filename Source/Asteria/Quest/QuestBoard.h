// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Quest.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Actor.h"
#include "QuestBoard.generated.h"

UCLASS()
class ASTERIA_API AQuestBoard : public AActor
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

	UPROPERTY(VisibleAnywhere)
	TArray<FQuest> QuestsPull;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent> QuestBoardMeshComp;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UWidgetComponent> QuestBoardWidgetComp;
	
	void AddQuestsPull(const FQuest& Quest);
};
