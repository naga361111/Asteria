// Copyright Epic Games, Inc. All Rights Reserved.

#include "UtilityBrainComponent.h"

#include "NpcActionDef.h"
#include "NeedsComponent.h"
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

	// 모든 행동에 욕구 대입 → Base + Σ(가중치 × 욕구) → argmax.
	FName Best = NAME_None;
	float BestScore = TNumericLimits<float>::Lowest();
	ActionTable->ForeachRow<FNpcActionDef>(TEXT("UtilityBrain"),
		[&](const FName& RowName, const FNpcActionDef& Row)
		{
			float Score = Row.BaseScore;
			for (const TPair<ENpcNeed, float>& Weight : Row.NeedWeights)
			{
				Score += Weight.Value * GetNeedValue(*Needs, Weight.Key);
			}
			if (Score > BestScore)
			{
				BestScore = Score;
				Best = RowName;
			}
		});

	ChosenAction = Best;
}
