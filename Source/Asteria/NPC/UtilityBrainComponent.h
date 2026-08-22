// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UtilityBrainComponent.generated.h"

class UDataTable;
class UActionRunnerComponent;

/** 효용 AI 평가기. AIController에 붙어, 빙의한 Body의 Needs를 읽어
 *  모든 행동을 점수화하고 최대값 행동을 고른다. 실행은 아직 없다(선택만 관측). */
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

	/** 고른 행동을 실행시킬 대상. 컨트롤러가 연결한다. (넘기는 로직은 아직 없음) */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Utility AI")
	TObjectPtr<UActionRunnerComponent> ActionRunner;
};
