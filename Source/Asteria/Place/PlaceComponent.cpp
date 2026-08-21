// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlaceComponent.h"

#include "PlacePointComponent.h"
#include "GameFramework/Actor.h"

TArray<FVector> UPlaceComponent::GetPoints(EPlacePoint Kind) const
{
	TArray<FVector> Out;

	const AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return Out;
	}

	TArray<UPlacePointComponent*> Points;
	Owner->GetComponents<UPlacePointComponent>(Points);
	for (const UPlacePointComponent* Point : Points)
	{
		if (Point != nullptr && Point->Kind == Kind)
		{
			Out.Add(Point->GetComponentLocation());
		}
	}
	return Out;
}
