// Copyright Epic Games, Inc. All Rights Reserved.

#include "NeedsComponent.h"

#include "UObject/Class.h" // UEnum

namespace
{
	/** enum의 모든 종류를 맵에 보장. 없는 키만 0으로 추가하고 기존 값은 보존한다. */
	void EnsureAllNeeds(TMap<ENpcNeed, int32>& Values)
	{
		const UEnum* NeedEnum = StaticEnum<ENpcNeed>();
		for (int32 i = 0; i < NeedEnum->NumEnums() - 1; ++i) // 마지막 자동 _MAX 항목 제외
		{
			Values.FindOrAdd(static_cast<ENpcNeed>(NeedEnum->GetValueByIndex(i)));
		}
	}
}

UNeedsComponent::UNeedsComponent()
{
	// 값만 보유한다. 틱 없음.
	PrimaryComponentTick.bCanEverTick = false;

	// CDO·BP 컴포넌트 템플릿에도 도니까 여기서 채워야 디테일 패널에 전 항목이 뜬다.
	EnsureAllNeeds(Values);
}

void UNeedsComponent::OnRegister()
{
	Super::OnRegister();

	// 생성자 이후 enum에 값이 추가된 옛 에셋(직렬화된 맵이 생성자를 덮어씀)을 런타임에 메꾼다.
	EnsureAllNeeds(Values);
}
