// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AsteriaHUD.generated.h"

class UUserWidget;

/**
 * 로컬 플레이어의 메인 HUD 위젯을 띄운다. AHUD는 로컬 플레이어에게만 생성되므로 플레이어별 화면 분리는 엔진이 보장한다.
 */
UCLASS()
class ASTERIA_API AAsteriaHUD : public AHUD
{
	GENERATED_BODY()

protected:
	// GameState가 이미 있으면 바로, 없으면(클라 초기 복제 전) 도착 신호를 기다렸다 메인 HUD를 만든다.
	virtual void BeginPlay() override;

	// 화면에 띄울 메인 HUD 위젯 클래스(WBP_MainHUD). BP에서 지정한다.
	UPROPERTY(EditDefaultsOnly, Category="HUD")
	TSubclassOf<UUserWidget> MainHUDClass;

private:
	// MainHUDClass 위젯을 소유 플레이어 컨트롤러로 생성해 뷰포트에 추가한다.
	void CreateMainHUD();
};
