// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/AsteriaNpc.h"

// Sets default values
AAsteriaNpc::AAsteriaNpc()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAsteriaNpc::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAsteriaNpc::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAsteriaNpc::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

