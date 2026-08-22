// Copyright Epic Games, Inc. All Rights Reserved.

#include "UtilityBrainComponent.h"

#include "NpcActionDef.h"
#include "NpcTemperamentDef.h"
#include "ActionRunnerComponent.h"
#include "NeedsComponent.h"
#include "TemperamentComponent.h"
#include "Time/AsteriaTimeSubsystem.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"

UUtilityBrainComponent::UUtilityBrainComponent()
{
	// 매 틱은 게임 분 게이트로만 쓴다(분이 바뀔 때만 스코어링). 재평가 클럭은 게임 시간.
	PrimaryComponentTick.bCanEverTick = true;
}

void UUtilityBrainComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ActionTable == nullptr)
	{
		return;
	}

	// Set 실행 중엔 재선택을 자제한다 — 완주 후 최신 수치로 새로 채점.
	// 긴급 중단이 필요해지면 여기에 예외 조건을 붙이면 된다(러너는 지금도 즉시 교체 가능).
	// 분 게이트보다 앞에 둬야 LastEvalMinute이 낡은 채 남아 idle 직후 바로 채점된다.
	if (ActionRunner != nullptr && ActionRunner->IsRunning())
	{
		return;
	}

	// 게임 분이 바뀐 프레임에만 재평가(허기 상승과 같은 클럭). 나머지 틱은 즉시 반환.
	const UWorld* World = GetWorld();
	const UAsteriaTimeSubsystem* Time = World ? World->GetSubsystem<UAsteriaTimeSubsystem>() : nullptr;
	if (Time == nullptr)
	{
		return;
	}
	const int32 NowMinute = FMath::FloorToInt(Time->GetTimeMinutes());
	if (NowMinute == LastEvalMinute)
	{
		return;
	}
	LastEvalMinute = NowMinute;

	// 뇌는 Controller에 붙고 욕구는 빙의한 Pawn(Body)에 있다.
	const AAIController* AI = Cast<AAIController>(GetOwner());
	const APawn* Body = AI ? AI->GetPawn() : nullptr;
	const UNeedsComponent* Needs = Body ? Body->FindComponentByClass<UNeedsComponent>() : nullptr;
	if (Needs == nullptr)
	{
		return;
	}

	// 기질: 욕구값에 곱해지는 영구 배율. 여러 기질은 욕구별로 곱 누적.
	// 기질 없는 욕구는 맵에 없음 → 조회 시 1.0 폴백(0.0이면 항을 죽임).
	TMap<ENpcNeed, float> NeedMult;
	const UTemperamentComponent* Temp = Body->FindComponentByClass<UTemperamentComponent>();
	if (Temp != nullptr && Temp->Table != nullptr)
	{
		for (const FName& Name : Temp->Temperaments)
		{
			const FNpcTemperamentDef* Row = Temp->Table->FindRow<FNpcTemperamentDef>(Name, TEXT("UtilityBrain"));
			if (Row == nullptr)
			{
				continue; // set엔 있지만 Table에서 사라진 이름 → 무시.
			}
			for (const TPair<ENpcNeed, float>& M : Row->Multipliers)
			{
				NeedMult.FindOrAdd(M.Key, 1.0f) *= M.Value;
			}
		}
	}

	// 모든 행동에 욕구 대입 → Base + Σ(가중치 × 욕구 × 기질배율) → argmax.
	FName Best = NAME_None;
	float BestScore = TNumericLimits<float>::Lowest();
	ActionTable->ForeachRow<FNpcActionDef>(TEXT("UtilityBrain"),
		[&](const FName& RowName, const FNpcActionDef& Row)
		{
			float Score = Row.BaseScore;
			for (const TPair<ENpcNeed, float>& Weight : Row.NeedWeights)
			{
				const float* Mult = NeedMult.Find(Weight.Key);
				Score += Weight.Value * Needs->Values.FindRef(Weight.Key) * (Mult ? *Mult : 1.0f);
			}
			if (Score > BestScore)
			{
				BestScore = Score;
				Best = RowName;
			}
		});

	// 여기까지 왔으면 러너는 idle — 고른 걸 항상 넘긴다(같은 행동이면 그대로 반복).
	if (ActionRunner != nullptr)
	{
		if (const FNpcActionDef* Row = ActionTable->FindRow<FNpcActionDef>(Best, TEXT("UtilityBrain")))
		{
			ActionRunner->RunSet(Row->ActionSet);
		}
	}

	ChosenAction = Best;
}
