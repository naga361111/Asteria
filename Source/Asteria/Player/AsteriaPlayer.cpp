// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AsteriaPlayer.h"

#include "DrawDebugHelpers.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EngineUtils.h"
#include "InputActionValue.h"
#include "Building/Common/BuildingGrid.h"
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
	// 자동 노출(밝기 적응) 끔. 밝기는 BP에서 카메라 Exposure Compensation으로 조정.
	CameraComp->PostProcessSettings.bOverride_AutoExposureMethod = true;
	CameraComp->PostProcessSettings.AutoExposureMethod = AEM_Manual;
	CameraComp->PostProcessSettings.bOverride_AutoExposureBias = true;
	CameraComp->PostProcessSettings.AutoExposureBias = 12.5f; // 실내 기준

	BoxComp = CreateDefaultSubobject<UBoxComponent>(FName("InteractionArea"));
	BoxComp->SetupAttachment(CameraComp);

	// 오버랩 이벤트를 실제로 발생시키려면 이게 켜져 있어야 한다.
	BoxComp->SetGenerateOverlapEvents(true);

	// 루트에 붙지만 위치는 매 프레임 변 위치로 직접 지정하므로 절대 트랜스폼.
	PreviewComp = CreateDefaultSubobject<UStaticMeshComponent>(FName("Preview"));
	PreviewComp->SetupAttachment(RootComponent);
	PreviewComp->SetUsingAbsoluteLocation(true);
	PreviewComp->SetUsingAbsoluteRotation(true);
	PreviewComp->SetUsingAbsoluteScale(true);
	PreviewComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewComp->SetCastShadow(false);
	PreviewComp->SetVisibility(false);

	// 피벗(변 중점 바닥)에서 앞(+Y)으로 100 떨어진 바닥. DecalSize.X는 투영 깊이, Y·Z는 화살표 반경.
	PreviewDecalComp = CreateDefaultSubobject<UDecalComponent>(FName("PreviewDecal"));
	PreviewDecalComp->SetupAttachment(PreviewComp);
	PreviewDecalComp->SetRelativeLocation(FVector(0.f, -100.f, 0.f));
	// 피치 -90으로 로컬 X(투영)를 아래로 돌린다. yaw는 PIE에서 화살표가 앞을 가리키도록 맞춘 값.
	PreviewDecalComp->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	PreviewDecalComp->DecalSize = FVector(50.f, 50.f, 50.f);
	PreviewDecalComp->SetVisibility(false);
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

	// 레벨의 첫 번째 격자.
	TActorIterator<ABuildingGrid> It(GetWorld());
	if (It)
	{
		Grid = *It;
	}
}

// Called every frame
void AAsteriaPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 시선 광선은 로컬 화면 기준이라 로컬 조종 중일 때만.
	if (!IsLocallyControlled()) return;

	// 격자·광선·변 중 하나라도 없으면 null로 미리보기를 숨긴다.
	FRay Ray;
	const FBuildingGridEdge* Edge = Grid.IsValid() && GetViewRay(Ray) ? Grid->FindNearestEdge(Ray) : nullptr;
	UpdatePreview(Edge, Ray.Origin);
	if (Edge == nullptr) return;

	// 변 Transform은 액터 기준이라 격자 트랜스폼을 곱해 월드로. 변은 로컬 X축 방향으로 CellSize.
	const FTransform World = Edge->Transform * Grid->GetActorTransform();
	const FVector Half(ABuildingGrid::CellSize * 0.5f, 0.f, 0.f);
	DrawDebugLine(GetWorld(), World.TransformPosition(-Half), World.TransformPosition(Half), FColor::Green);
}

void AAsteriaPlayer::UpdatePreview(const FBuildingGridEdge* Edge, const FVector& ViewOrigin)
{
	// 재질이 없으면 원래 재질 그대로 보이므로 띄우지 않는다.
	if (Edge == nullptr || !PlaceableMeshes.IsValidIndex(SelectedMeshIndex) || PreviewMaterial == nullptr)
	{
		// 자식인 PreviewDecalComp도 함께 숨긴다.
		PreviewComp->SetVisibility(false, true);
		return;
	}

	// 메시마다 슬롯 수가 다르므로 메시를 넣은 뒤 그 슬롯 수만큼 덮어쓴다.
	PreviewComp->SetStaticMesh(PlaceableMeshes[SelectedMeshIndex]);
	for (int32 Index = 0; Index < PreviewComp->GetNumMaterials(); ++Index)
	{
		PreviewComp->SetMaterial(Index, PreviewMaterial);
	}

	// SpawnEdgeMeshes()가 인스턴스를 놓는 위치와 같다.
	PreviewComp->SetWorldTransform(ABuildingGrid::GetEdgeMeshTransform(*Edge, ShouldFlip(*Edge, ViewOrigin)) * Grid->GetActorTransform());
	PreviewComp->SetVisibility(true, true);
}

bool AAsteriaPlayer::ShouldFlip(const FBuildingGridEdge& Edge, const FVector& ViewOrigin) const
{
	// 뒤집지 않은 메시의 월드 트랜스폼. 피벗이 변 중점이고 앞은 로컬 -Y.
	const FTransform World = ABuildingGrid::GetEdgeMeshTransform(Edge, false) * Grid->GetActorTransform();
	const FVector Front = -World.GetUnitAxis(EAxis::Y);

	// 앞이 시점 반대편이면 자동으로 뒤집고, R(bPlaceFlipped)로 한 번 더 뒤집는다.
	const bool bAutoFlip = FVector::DotProduct(Front, ViewOrigin - World.GetLocation()) < 0.f;
	return bAutoFlip != bPlaceFlipped;
}

bool AAsteriaPlayer::GetViewRay(FRay& OutRay) const
{
	const APlayerController* PC = GetController<APlayerController>();
	if (PC == nullptr) return false;

	// 조준점(WBP_PlayerHUD)은 화면 중앙이라 뷰포트 중앙 픽셀을 역투영한다.
	int32 SizeX = 0;
	int32 SizeY = 0;
	PC->GetViewportSize(SizeX, SizeY);

	FVector Origin;
	FVector Direction;
	if (!PC->DeprojectScreenPositionToWorld(SizeX * 0.5f, SizeY * 0.5f, Origin, Direction)) return false;

	OutRay = FRay(Origin, Direction, true);
	return true;
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

void AAsteriaPlayer::SelectMesh(const FInputActionValue& Value)
{
	const int32 Num = PlaceableMeshes.Num();
	if (Num == 0) return;

	const float Axis = Value.Get<float>();
	if (Axis == 0.f) return;

	const int32 Step = Axis > 0.f ? 1 : -1;
	SelectedMeshIndex = (SelectedMeshIndex + Step + Num) % Num;
}

void AAsteriaPlayer::Place(const FInputActionValue& Value)
{
	if (!Grid.IsValid() || !PlaceableMeshes.IsValidIndex(SelectedMeshIndex)) return;

	FRay Ray;
	if (!GetViewRay(Ray)) return;

	const FBuildingGridEdge* Edge = Grid->FindNearestEdge(Ray);
	if (Edge == nullptr) return;

	Grid->SetEdgeMesh(Edge->Vertex, Edge->Axis, PlaceableMeshes[SelectedMeshIndex], ShouldFlip(*Edge, Ray.Origin));
}

void AAsteriaPlayer::FlipPlacement(const FInputActionValue& Value)
{
	bPlaceFlipped = !bPlaceFlipped;
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
		EIC->BindAction(SelectAction, ETriggerEvent::Triggered, this, &AAsteriaPlayer::SelectMesh);
		EIC->BindAction(PlaceAction, ETriggerEvent::Started, this, &AAsteriaPlayer::Place);
		EIC->BindAction(FlipAction, ETriggerEvent::Started, this, &AAsteriaPlayer::FlipPlacement);
	}
}
