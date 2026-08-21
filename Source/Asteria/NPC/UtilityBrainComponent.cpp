// Copyright Epic Games, Inc. All Rights Reserved.

#include "UtilityBrainComponent.h"

#include "NpcActionDef.h"
#include "NpcTemperamentDef.h"
#include "ActionRunnerComponent.h"
#include "NeedsComponent.h"
#include "TemperamentComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Engine/DataTable.h"

namespace
{
	int32 GetNeedValue(const UNeedsComponent& Needs, ENpcNeed Need)
	{
		switch (Need)
		{
		case ENpcNeed::Survival: return Needs.Survival;
		case ENpcNeed::Safety:   return Needs.Safety;
		case ENpcNeed::Duty:     return Needs.Duty;
		case ENpcNeed::Social:   return Needs.Social;
		default:                 return 0;
		}
	}
}

UUtilityBrainComponent::UUtilityBrainComponent()
{
	// 매 틱 재평가(관측용). N초 타이머·히스테리시스는 나중에.
	PrimaryComponentTick.bCanEverTick = true;
}

void UUtilityBrainComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ActionTable == nullptr)
	{
		return;
	}

	// 뇌는 Controller에 붙고 욕구는 빙의한 Pawn(Body)에 있다.
	const AAIController* AI = Cast<AAIController>(GetOwner());
	const APawn* Body = AI ? AI->GetPawn() : nullptr;
	const UNeedsComponent* Needs = Body ? Body->FindComponentByClass<UNeedsComponent>() : nullptr;
	if (Needs == nullptr)
	{
		return;
	}

	// 기질: 욕구값에 곱해지는 영구 배율. 여러 기질은 욕구별로 곱 누적.
	// 없음/Table null/빈 set → 전부 1.0(평범)이라 현행 결과와 동일.
	float NeedMult[4] = { 1.f, 1.f, 1.f, 1.f };
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
				NeedMult[static_cast<uint8>(M.Key)] *= M.Value;
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
				Score += Weight.Value * GetNeedValue(*Needs, Weight.Key) * NeedMult[static_cast<uint8>(Weight.Key)];
			}
			if (Score > BestScore)
			{
				BestScore = Score;
				Best = RowName;
			}
		});

	// 선택이 바뀔 때만 러너에 실행을 넘긴다(매 틱 넘기면 트리가 계속 재시작).
	if (Best != ChosenAction && ActionRunner != nullptr)
	{
		if (const FNpcActionDef* Row = ActionTable->FindRow<FNpcActionDef>(Best, TEXT("UtilityBrain")))
		{
			ActionRunner->RunSet(Row->ActionSet, Row->TargetLocation);
		}
	}

	ChosenAction = Best;
}
