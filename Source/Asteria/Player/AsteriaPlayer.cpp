// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AsteriaPlayer.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

// Sets default values
AAsteriaPlayer::AAsteriaPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// First person: the controller drives yaw/pitch so aiming rotates the view.
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = true;
	bUseControllerRotationRoll = false;
	
	CameraComp = CreateDefaultSubobject<UCameraComponent>(FName("Camera"));
	CameraComp->SetupAttachment(RootComponent);
	CameraComp->bUsePawnControlRotation = true;
	CameraComp->SetRelativeLocation(FVector(20.0f, 0.0f, 70.0f));
	
	BoxComp = CreateDefaultSubobject<UBoxComponent>(FName("InteractionArea"));
	BoxComp->SetupAttachment(CameraComp);
}

// Called when the game starts or when spawned
void AAsteriaPlayer::BeginPlay()
{
	Super::BeginPlay();

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

// Called every frame
void AAsteriaPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAsteriaPlayer::Move(const FInputActionValue& Value)
{
	// First person: actor already tracks controller yaw, so its own axes are the aim direction.
	const FVector2D Axis = Value.Get<FVector2D>();
	AddMovementInput(GetActorForwardVector(), Axis.Y);
	AddMovementInput(GetActorRightVector(), Axis.X);
}

void AAsteriaPlayer::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(Axis.Y);
}

// Called to bind functionality to input
void AAsteriaPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAsteriaPlayer::Move);
		}
		if (LookAction)
		{
			EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAsteriaPlayer::Look);
		}
	}
}
