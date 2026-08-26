// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AsteriaPlayer.h"

// Sets default values
AAsteriaPlayer::AAsteriaPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAsteriaPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAsteriaPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAsteriaPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

