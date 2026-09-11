// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CounterBoardWidget.generated.h"

class AAsteriaGameState;
class AGameStateBase;
class UCounterService;
class UTileView;

/**
 * CounterService의 제출함(SubmittedClaimIds)을 TileView로 표시하는 컨테이너. 표시 전용 소비자.
 * 상태는 서버가 소유하고 이 위젯은 복제된 값을 읽기만 한다 — 여기서 목록을 고치지 않는다.
 */
UCLASS()
class ASTERIA_API UCounterBoardWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// GameState는 복제 액터라 클라에선 이 위젯보다 늦게 도착할 수 있다.
	// 있으면 즉시 붙고, 없으면 도착 신호를 기다렸다 붙는다.
	void BindToCounterService(AAsteriaGameState* GameState);
	void HandleGameStateSet(AGameStateBase* NewGameState);

	// 제출함을 읽어 TileView를 다시 그림 (bind 대상 + 초기 prime)
	void RefreshSubmissions();

	// WBP의 TileView와 이름 일치시킬 것
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTileView> SubmissionTileView;

private:
	// 구독 해제용. 위젯이 서비스보다 먼저 죽어도 바인딩을 남기지 않는다.
	TWeakObjectPtr<UCounterService> BoundService;
	FDelegateHandle GameStateSetHandle;
};
