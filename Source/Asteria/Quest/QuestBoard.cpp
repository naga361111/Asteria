// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/QuestBoard.h"

#include "UI/QuestBoardTileViewWidget.h"

// Sets default values
AQuestBoard::AQuestBoard()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	QuestBoardMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(FName("QuestMeshComponent"));
	QuestBoardMeshComp->SetupAttachment(RootComponent);

	QuestBoardWidgetComp = CreateDefaultSubobject<UWidgetComponent>(FName("QuestBoardWidgetComponent"));
	QuestBoardWidgetComp->SetupAttachment(QuestBoardMeshComp);
}

// Called when the game starts or when spawned
void AQuestBoard::BeginPlay()
{
	Super::BeginPlay();

	UUserWidget* UW = QuestBoardWidgetComp->GetWidget();
	UQuestBoardTileViewWidget* QBTVW = Cast<UQuestBoardTileViewWidget>(UW);
	if (QBTVW != nullptr)
	{
		QBTVW->QuestBoard = this;
		QBTVW->Refresh();
	}
}

// Called every frame
void AQuestBoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AQuestBoard::AddQuestsPull(const FQuest& Quest)
{
	QuestsPull.Add(Quest);
	
	UUserWidget* UW = QuestBoardWidgetComp->GetWidget();
	UQuestBoardTileViewWidget* QBTVW = Cast<UQuestBoardTileViewWidget>(UW);
	if (QBTVW != nullptr)
	{
		QBTVW->Refresh();
	}
}
