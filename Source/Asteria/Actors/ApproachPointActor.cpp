// Fill out your copyright notice in the Description page of Project Settings.


#include "ApproachPointActor.h"


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

