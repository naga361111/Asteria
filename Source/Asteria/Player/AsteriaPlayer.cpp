// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AsteriaPlayer.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Interaction/Interactable.h"
#include "GameState/AsteriaGameState.h"
#include "GameState/Components/CounterService.h"

void AAsteriaPlayer::Server_AcceptQuestAssignment_Implementation(int32 AssignmentId)
{
	AAsteriaGameState* GameState = GetWorld()->GetGameState<AAsteriaGameState>();
	UCounterService* CounterService = GameState ? GameState->CounterService : nullptr;
	if (CounterService == nullptr) return;

	// 여기는 전달만 한다. 실재·상태 검증은 소유자(QuestService)가 하고,
	// 클라가 보낸 AssignmentId가 stale이면 그쪽에서 조용히 거절된다.
	//
	// TODO: "이 플레이어가 실제로 창구 앞에 있는가"는 아직 못 막는다.
	// AAsteriaPlayer::OverlappedActor는 IsLocallyControlled 경로에서만 채워져 서버엔 없고,
	// 창구를 식별할 타입도 아직 없다(BP_CounterActor는 C++ 타입이 없다).
	// 상호작용 대상을 서버가 아는 계약이 서면 그때 여기서 막는다.
	CounterService->AcceptQuestAssignment(AssignmentId);
}

void AAsteriaPlayer::Server_SettleQuestAssignment_Implementation(int32 AssignmentId)
{
	AAsteriaGameState* GameState = GetWorld()->GetGameState<AAsteriaGameState>();
	UCounterService* CounterService = GameState ? GameState->CounterService : nullptr;
	if (CounterService == nullptr) return;

	// 수락과 같은 전달 전용 경로. 검증은 소유자(QuestService)가 한 벌로 한다.
	// 창구 앞에 있는지 못 막는 것도 위와 같다.
	CounterService->SettleQuestAssignment(AssignmentId);
}

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
	if (!IsLocallyControlled()) return;

	if (OtherActor != nullptr && OtherActor != this && OtherActor->Implements<UInteractable>())
	{
		OverlappedActor = OtherActor;
	}
}

void AAsteriaPlayer::OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsLocallyControlled()) return;

	if (OtherActor == OverlappedActor)
	{
		OverlappedActor = nullptr;

		// 범위를 벗어나면 열려 있던 UI 모드를 무조건 닫는다.
		SetUIInputMode(false);
	}
}

void AAsteriaPlayer::SetUIInputMode(bool bEnable)
{
	// 입력 모드/커서는 로컬 플레이어의 뷰포트 상태다. 서버나 원격 프록시에서 건드리지 않는다.
	if (!IsLocallyControlled()) return;

	APlayerController* PC = GetController<APlayerController>();
	if (PC == nullptr) return;

	bUIInputMode = bEnable;
	PC->bShowMouseCursor = bEnable;

	if (bEnable)
	{
		FInputModeGameAndUI Mode;
		// 캡처될 때만 뷰포트에 가두고, 캡처 중에도 커서를 계속 보여준다.
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::LockOnCapture);
		Mode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(Mode);
	}
	else
	{
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
	if (!IsLocallyControlled() || !OverlappedActor.IsValid()) return;

	IInteractable* Target = Cast<IInteractable>(OverlappedActor.Get());
	if (Target && Target->CanInteract())
		Target->OnInteract(this);
	
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
