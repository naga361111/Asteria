// Copyright Epic Games, Inc. All Rights Reserved.

#include "UtilityBrainComponent.h"

UUtilityBrainComponent::UUtilityBrainComponent()
{
	// 수치 시스템 개편 중 — 행동 선택(스코어링) 제거. 지금은 데이터만 보유, 아무것도 안 고른다.
	PrimaryComponentTick.bCanEverTick = false;
}
