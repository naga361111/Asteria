// Copyright Epic Games, Inc. All Rights Reserved.

#include "NeedsComponent.h"

UNeedsComponent::UNeedsComponent()
{
	// 값만 보유한다. 틱 없음.
	PrimaryComponentTick.bCanEverTick = false;
}
