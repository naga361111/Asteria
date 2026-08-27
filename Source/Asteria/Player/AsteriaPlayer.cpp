// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AsteriaPlayer.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Interaction/Interactable.h"

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

	// 오버랩 이벤트를 실제로 발생시키려면 이게 켜져 있어야 한다.
	BoxComp->SetGenerateOverlapEvents(true);
}

void AAsteriaPlayer::OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                             const FHitResult& SweepResult)
{
	if (OtherActor != nullptr && OtherActor != this && OtherActor->Implements<UInteractable>())
	{
		OverlappedActor = OtherActor;
	}
}

void AAsteriaPlayer::OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == OverlappedActor)
	{
		OverlappedActor = nullptr;
		
		APlayerController* PC = GetController<APlayerController>();
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}
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

	// --- 델리게이트 바인딩 ---
	// AddDynamic(수신 객체, &클래스::핸들러). 핸들러는 위에서 선언한 UFUNCTION들.
	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &AAsteriaPlayer::OnDetectionBeginOverlap);
	BoxComp->OnComponentEndOverlap.AddDynamic(this, &AAsteriaPlayer::OnDetectionEndOverlap);
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

void AAsteriaPlayer::Interact(const FInputActionValue& Value)
{
	if (OverlappedActor.IsValid())
	{
		APlayerController* PC = GetController<APlayerController>();
		PC->bShowMouseCursor = !PC->bShowMouseCursor;
		PC->SetInputMode(FInputModeGameAndUI());
	}
}

// Called to bind functionality to input
void AAsteriaPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAsteriaPlayer::Move);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAsteriaPlayer::Look);
		EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &AAsteriaPlayer::Interact);
	}
}
