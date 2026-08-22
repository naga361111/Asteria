// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UtilityBrainComponent.generated.h"

class UDataTable;
class UActionRunnerComponent;

/** 효용 AI 평가기. AIController에 붙어, 빙의한 Body의 Needs를 읽어
 *  모든 행동을 점수화하고 최대값 행동을 고른다. 선택이 바뀌면 러너에 실행을 넘긴다.
 *  재평가는 게임 분이 바뀔 때만(AsteriaTimeSubsystem 기준) — 허기 상승과 같은 클럭. */
UCLASS(ClassGroup=(NPC), meta=(BlueprintSpawnableComponent))
class ASTERIA_API UUtilityBrainComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UUtilityBrainComponent();

	/** 행동 정의 테이블(FNpcActionDef 행). 에디터에서 지정. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Utility AI")
	TObjectPtr<UDataTable> ActionTable;

	/** 이번 평가에서 선택된 행동의 행 이름. 디버거가 읽는다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Utility AI")
	FName ChosenAction;

	/** 고른 행동을 실행시킬 대상. 컨트롤러가 연결한다. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Utility AI")
	TObjectPtr<UActionRunnerComponent> ActionRunner;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

private:
	/** 마지막으로 스코어링한 게임 분(정수). 이 값이 바뀔 때만 재평가한다. */
	int32 LastEvalMinute = -1;
};
