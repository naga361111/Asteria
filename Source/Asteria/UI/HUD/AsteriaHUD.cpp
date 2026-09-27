// Fill out your copyright notice in the Description page of Project Settings.


#include "AsteriaHUD.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"

void AAsteriaHUD::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (World->GetGameState())
	{
		CreateMainHUD();
	}
	else
	{
		// 아직 복제 전. 도착하면 한 번만 만들고 구독을 푼다.
		World->GameStateSetEvent.AddWeakLambda(this, [this, World](AGameStateBase*)
		{
			World->GameStateSetEvent.RemoveAll(this);
			CreateMainHUD();
		});
	}
}

void AAsteriaHUD::CreateMainHUD()
{
	if (!MainHUDClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("AsteriaHUD: no MainHUDClass. Main HUD not created."));
		return;
	}

	if (UUserWidget* MainHUD = CreateWidget<UUserWidget>(GetOwningPlayerController(), MainHUDClass))
	{
		// 표시 전용. 자식까지 클릭을 막지 않아야 월드 위젯이 클릭을 받는다.
		MainHUD->SetVisibility(ESlateVisibility::HitTestInvisible);
		MainHUD->AddToViewport();
	}
}
