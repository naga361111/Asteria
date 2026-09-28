// Fill out your copyright notice in the Description page of Project Settings.


#include "ApproachPointActor.h"

#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AApproachPointActor::AApproachPointActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AApproachPointActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AApproachPointActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

AApproachPointActor* AApproachPointActor::FindNearestFree(UWorld* World, EApproachPointType Type, const FVector& Origin)
{
	TArray<AActor*> Points;
	UGameplayStatics::GetAllActorsOfClass(World, AApproachPointActor::StaticClass(), Points);

	AApproachPointActor* Best = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	for (AActor* Actor : Points)
	{
		AApproachPointActor* Point = Cast<AApproachPointActor>(Actor);
		if (Point == nullptr || Point->PointType != Type || Point->Occupant.IsValid())
		{
			continue;
		}

		const float DistSq = FVector::DistSquared2D(Origin, Point->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Point;
		}
	}
	return Best;
}

