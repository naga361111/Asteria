// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "AsteriaPlayer.generated.h"

struct FInputActionValue;

UCLASS()
class ASTERIA_API AAsteriaPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAsteriaPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Input mapping context applied to this player on possession.
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;

	// Move action (2D: forward/back + right/left).
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	// Aim/Look action (2D: yaw + pitch).
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact(const FInputActionValue& Value);
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCameraComponent> CameraComp;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UBoxComponent> BoxComp;
	
	// OnComponentBeginOverlap 델리게이트 시그니처와 1:1 대응. UFUNCTION() 필수(동적 델리게이트라 리플렉션 필요).
	UFUNCTION()
	void OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// OnComponentEndOverlap 델리게이트 시그니처. Begin과 달리 bFromSweep/SweepResult 인자가 없다.
	UFUNCTION()
	void OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	TWeakObjectPtr<AActor> OverlappedActor;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
