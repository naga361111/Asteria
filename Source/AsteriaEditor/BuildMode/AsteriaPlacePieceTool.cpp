// Fill out your copyright notice in the Description page of Project Settings.

#include "BuildMode/AsteriaPlacePieceTool.h"

#include "BuildMode/AsteriaGridSnap.h"

#include "BaseBehaviors/MouseHoverBehavior.h"
#include "BaseBehaviors/SingleClickBehavior.h"
#include "BaseBehaviors/SingleKeyCaptureBehavior.h"
#include "Components/StaticMeshComponent.h"
#include "Editor.h"
#include "Engine/StaticMesh.h"
#include "Engine/HitResult.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "InteractiveToolManager.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "PrimitiveDrawingUtils.h"
#include "SceneManagement.h"
#include "ScopedTransaction.h"
#include "ToolContextInterfaces.h"
#include "UObject/ConstructorHelpers.h"

#define LOCTEXT_NAMESPACE "AsteriaPlacePieceTool"

namespace
{
	// 엔진에 이미 있는 언릿 반투명 머티리얼. "Color" 벡터 파라미터 하나로 색·불투명도가 정해진다.
	// 검증(초록/빨강) 단계에서 프로젝트 전용 고스트 머티리얼로 갈아끼울 자리다.
	const TCHAR* DefaultGhostMaterialPath = TEXT("/Engine/EngineDebugMaterials/M_SimpleUnlitTranslucent.M_SimpleUnlitTranslucent");
	const FName GhostColorParameterName(TEXT("Color"));

	constexpr int FloorUpModifierId = 1;
	constexpr int FloorDownModifierId = 2;
	constexpr int RemoveModifierId = 3;

	// 이 이상이면 "윗면"으로 본다.
	constexpr double UpFacingNormalThreshold = 0.5;

	// 벽면이 격자선 위에 정확히 놓였을 때 반올림 타이를 깨는 값(cm).
	constexpr double GridTieBreakEpsilon = 0.5;
}

UAsteriaPlacePieceToolProperties::UAsteriaPlacePieceToolProperties()
{
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GhostMaterialFinder(DefaultGhostMaterialPath);
	PreviewMaterial = GhostMaterialFinder.Object;
}

bool UAsteriaPlacePieceToolBuilder::CanBuildTool(const FToolBuilderState& SceneState) const
{
	return SceneState.World != nullptr;
}

UInteractiveTool* UAsteriaPlacePieceToolBuilder::BuildTool(const FToolBuilderState& SceneState) const
{
	UAsteriaPlacePieceTool* Tool = NewObject<UAsteriaPlacePieceTool>(SceneState.ToolManager);
	Tool->SetTargetWorld(SceneState.World);
	return Tool;
}

void UAsteriaPlacePieceTool::SetTargetWorld(UWorld* InWorld)
{
	TargetWorld = InWorld;
}

void UAsteriaPlacePieceTool::Setup()
{
	UInteractiveTool::Setup();

	Settings = NewObject<UAsteriaPlacePieceToolProperties>(this);
	Settings->RestoreProperties(this);
	AddToolPropertySource(Settings);

	USingleClickInputBehavior* ClickBehavior = NewObject<USingleClickInputBehavior>(this);
	ClickBehavior->Initialize(this);
	// Shift는 클릭과 호버 양쪽에 등록한다. 클릭에만 달면 버튼을 누르기 전까지 프리뷰가 안 바뀐다.
	ClickBehavior->Modifiers.RegisterModifier(RemoveModifierId, FInputDeviceState::IsShiftKeyDown);
	AddInputBehavior(ClickBehavior);

	UMouseHoverBehavior* HoverBehavior = NewObject<UMouseHoverBehavior>(this);
	HoverBehavior->Initialize(this);
	HoverBehavior->Modifiers.RegisterModifier(RemoveModifierId, FInputDeviceState::IsShiftKeyDown);
	AddInputBehavior(HoverBehavior);

	// IClickBehaviorTarget과 IHoverBehaviorTarget이 IModifierToggleBehaviorTarget을 각각 상속하므로
	// this의 암시적 변환이 모호하다. 한쪽 경로를 명시한다 — 어느 쪽이든 같은 오버라이드로 디스패치된다.
	IModifierToggleBehaviorTarget* ModifierTarget = static_cast<IClickBehaviorTarget*>(this);

	USingleKeyCaptureBehavior* FloorUpBehavior = NewObject<USingleKeyCaptureBehavior>(this);
	FloorUpBehavior->Initialize(ModifierTarget, FloorUpModifierId, EKeys::PageUp);
	AddInputBehavior(FloorUpBehavior);

	USingleKeyCaptureBehavior* FloorDownBehavior = NewObject<USingleKeyCaptureBehavior>(this);
	FloorDownBehavior->Initialize(ModifierTarget, FloorDownModifierId, EKeys::PageDown);
	AddInputBehavior(FloorDownBehavior);

	CreatePreview();
	RefreshPreviewAppearance();

	GetToolManager()->DisplayMessage(
		LOCTEXT("ToolUsage", "클릭: 배치 · Shift+클릭: 커서가 가리킨 액터 제거 · PageUp/PageDown: 층 오프셋"),
		EToolMessageLevel::UserNotification);
}

void UAsteriaPlacePieceTool::Shutdown(EToolShutdownType ShutdownType)
{
	DestroyPreview();

	Settings->SaveProperties(this);

	UInteractiveTool::Shutdown(ShutdownType);
}

void UAsteriaPlacePieceTool::OnPropertyModified(UObject* PropertySet, FProperty* Property)
{
	RefreshPreviewAppearance();
}

FInputRayHit UAsteriaPlacePieceTool::IsHitByClick(const FInputDeviceRay& ClickPos)
{
	if (bRemoveModifierDown)
	{
		FHitResult Hit;
		if (!TraceWorld(ClickPos, Hit) || Hit.GetActor() == nullptr)
		{
			return FInputRayHit();
		}

		return FInputRayHit(Hit.Distance);
	}

	FTransform Placement;
	if (!ComputePlacement(ClickPos, Placement))
	{
		return FInputRayHit();
	}

	return FInputRayHit(FVector::Dist(ClickPos.WorldRay.Origin, Placement.GetLocation()));
}

FInputRayHit UAsteriaPlacePieceTool::BeginHoverSequenceHitTest(const FInputDeviceRay& PressPos)
{
	// 고스트가 떠야 할 조건과 클릭이 먹어야 할 조건은 같다.
	return IsHitByClick(PressPos);
}

void UAsteriaPlacePieceTool::OnBeginHover(const FInputDeviceRay& DevicePos)
{
	OnUpdateHover(DevicePos);
}

bool UAsteriaPlacePieceTool::OnUpdateHover(const FInputDeviceRay& DevicePos)
{
	LastHoverRay = DevicePos;
	bHasLastHoverRay = true;

	if (bRemoveModifierDown)
	{
		// 제거 모드에서는 고스트도 그리드도 안 띄운다. 지워질 액터만 표시한다.
		bHasHoverSnap = false;
		if (PreviewComponent != nullptr)
		{
			PreviewComponent->SetVisibility(false);
		}

		FHitResult Hit;
		AActor* Target = TraceWorld(DevicePos, Hit) ? Hit.GetActor() : nullptr;
		RemoveTargetActor = Target;

		return Target != nullptr;
	}

	RemoveTargetActor = nullptr;

	FTransform SnappedTransform;
	if (!ComputePlacement(DevicePos, SnappedTransform))
	{
		OnEndHover();
		return false;
	}

	// 고스트와 그리드는 여기서 한 번 계산한 값을 공유한다.
	HoverSnapLocation = SnappedTransform.GetLocation();
	bHasHoverSnap = true;

	if (PreviewComponent != nullptr)
	{
		const bool bShowGhost = (Settings->Piece != nullptr);
		if (bShowGhost)
		{
			PreviewComponent->SetWorldTransform(SnappedTransform);
		}
		PreviewComponent->SetVisibility(bShowGhost);
	}

	return true;
}

void UAsteriaPlacePieceTool::OnEndHover()
{
	bHasHoverSnap = false;
	RemoveTargetActor = nullptr;

	if (PreviewComponent != nullptr)
	{
		PreviewComponent->SetVisibility(false);
	}
}

void UAsteriaPlacePieceTool::Render(IToolsContextRenderAPI* RenderAPI)
{
	FPrimitiveDrawInterface* PDI = RenderAPI->GetPrimitiveDrawInterface();
	if (PDI == nullptr)
	{
		return;
	}

	if (const AActor* Target = RemoveTargetActor.Get())
	{
		DrawWireBox(PDI, Target->GetComponentsBoundingBox(/*bNonColliding=*/true),
			Settings->RemoveHighlightColor, SDPG_Foreground, 2.f, 0.f, /*bScreenSpace=*/true);
		return;
	}

	if (!bHasHoverSnap || !Settings->bDrawGrid || Settings->GridSize <= 0.f)
	{
		return;
	}

	const double CellSize = static_cast<double>(Settings->GridSize);
	const int32 Radius = Settings->GridDrawRadius;
	const double Extent = CellSize * Radius;

	// 스냅 결과가 격자 교점이므로, 커서 위치를 지나는 선이 곧 실제 스냅 후보선이다.
	for (int32 Index = -Radius; Index <= Radius; ++Index)
	{
		// 바깥으로 갈수록 옅게. 안 그러면 뷰포트가 선으로 덮인다.
		const float Fade = 1.f - static_cast<float>(FMath::Abs(Index)) / static_cast<float>(Radius + 1);

		FLinearColor LineColor = Settings->GridColor;
		LineColor.A *= Fade;

		const double Offset = CellSize * Index;

		PDI->DrawLine(
			HoverSnapLocation + FVector(Offset, -Extent, 0.0),
			HoverSnapLocation + FVector(Offset, Extent, 0.0),
			LineColor, SDPG_Foreground, 1.f, 0.f, /*bScreenSpace=*/true);

		PDI->DrawLine(
			HoverSnapLocation + FVector(-Extent, Offset, 0.0),
			HoverSnapLocation + FVector(Extent, Offset, 0.0),
			LineColor, SDPG_Foreground, 1.f, 0.f, /*bScreenSpace=*/true);
	}
}

void UAsteriaPlacePieceTool::OnClicked(const FInputDeviceRay& ClickPos)
{
	UWorld* World = TargetWorld.Get();
	if (World == nullptr)
	{
		return;
	}

	if (bRemoveModifierDown)
	{
		RemoveActorUnderCursor(ClickPos);
		return;
	}

	if (Settings->Piece == nullptr)
	{
		GetToolManager()->DisplayMessage(
			LOCTEXT("NoPieceAssigned", "배치할 Piece 메시가 지정되지 않았다."),
			EToolMessageLevel::UserWarning);
		return;
	}

	FTransform SpawnTransform;
	if (!ComputePlacement(ClickPos, SpawnTransform))
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("PlacePiece", "Place Build Piece"));

	FActorSpawnParameters SpawnParams;
	SpawnParams.ObjectFlags = RF_Transactional;

	AStaticMeshActor* Placed = World->SpawnActor<AStaticMeshActor>(
		SpawnTransform.GetLocation(), SpawnTransform.Rotator(), SpawnParams);
	if (Placed == nullptr)
	{
		return;
	}

	UStaticMeshComponent* MeshComponent = Placed->GetStaticMeshComponent();

	// Static 모빌리티 컴포넌트는 스폰 이후 메시 교체가 막혀 있다. 잠깐 풀었다가 되돌린다.
	MeshComponent->SetMobility(EComponentMobility::Movable);
	MeshComponent->SetStaticMesh(Settings->Piece);
	MeshComponent->SetMobility(EComponentMobility::Static);

	Placed->SetActorLabel(Settings->Piece->GetName());
}

void UAsteriaPlacePieceTool::CreatePreview()
{
	UWorld* World = TargetWorld.Get();
	if (World == nullptr)
	{
		return;
	}

	// 임시 에디터 액터: 레벨에 저장되지 않고 아웃라이너에도 뜨지 않는다.
	FActorSpawnParameters SpawnParams;
	SpawnParams.ObjectFlags = RF_Transient;
	SpawnParams.bTemporaryEditorActor = true;
	SpawnParams.bHideFromSceneOutliner = true;
	SpawnParams.bNoFail = true;

	PreviewActor = World->SpawnActor<AActor>(SpawnParams);
	if (PreviewActor == nullptr)
	{
		return;
	}

	PreviewComponent = NewObject<UStaticMeshComponent>(PreviewActor, NAME_None, RF_Transient);
	PreviewActor->SetRootComponent(PreviewComponent);
	PreviewComponent->SetMobility(EComponentMobility::Movable);
	// 충돌을 끄지 않으면 고스트가 자기 자신의 커서 트레이스를 가로막는다.
	PreviewComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewComponent->SetCastShadow(false);
	PreviewComponent->bSelectable = false;
	PreviewComponent->RegisterComponent();

	PreviewComponent->SetVisibility(false);
}

void UAsteriaPlacePieceTool::DestroyPreview()
{
	if (PreviewActor != nullptr)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}

	PreviewComponent = nullptr;
	PreviewMaterialInstance = nullptr;
}

void UAsteriaPlacePieceTool::RefreshPreviewAppearance()
{
	if (PreviewComponent == nullptr)
	{
		return;
	}

	if (PreviewComponent->GetStaticMesh() != Settings->Piece)
	{
		PreviewComponent->SetStaticMesh(Settings->Piece);
	}

	if (Settings->Piece == nullptr)
	{
		PreviewComponent->SetVisibility(false);
		return;
	}

	UMaterialInterface* GhostMaterial = Settings->PreviewMaterial;
	if (GhostMaterial == nullptr)
	{
		// 고스트 머티리얼을 비우면 메시 원본 머티리얼로 되돌린다.
		PreviewComponent->EmptyOverrideMaterials();
		PreviewMaterialInstance = nullptr;
		return;
	}

	if (PreviewMaterialInstance == nullptr || PreviewMaterialInstance->Parent != GhostMaterial)
	{
		PreviewMaterialInstance = UMaterialInstanceDynamic::Create(GhostMaterial, this);
	}

	PreviewMaterialInstance->SetVectorParameterValue(GhostColorParameterName, Settings->PreviewColor);

	const int32 MaterialCount = PreviewComponent->GetNumMaterials();
	for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
	{
		PreviewComponent->SetMaterial(MaterialIndex, PreviewMaterialInstance);
	}
}

double UAsteriaPlacePieceTool::ResolveStackZ(const FHitResult& Hit, double FloorHeight) const
{
	const AActor* HitActor = Hit.GetActor();
	if (HitActor == nullptr)
	{
		return static_cast<double>(Settings->GroundPlaneZ);
	}

	// 맞은 액터가 "놓인 층". 우리가 배치한 피스면 이미 층 배수라 그대로 나오고, 아니면 가장 가까운
	// 층으로 맞춘다. 어느 쪽이든 액터당 상수라, 커서가 그 면 위를 훑어도 값이 흔들리지 않는다.
	const double ActorFloorZ = FMath::GridSnap(HitActor->GetActorLocation().Z, FloorHeight);

	// 그 액터가 몇 층을 차지하는지 세서 그 위에 올린다. 바닥 슬래브처럼 층고보다 한참 낮은 피스는
	// 0층을 차지하므로 같은 층에 그대로 얹힌다 — 그 위에 벽이 선다.
	const FBox ActorBounds = HitActor->GetComponentsBoundingBox(/*bNonColliding=*/true);
	const double OccupiedHeight = ActorBounds.Max.Z - ActorFloorZ;
	const int32 OccupiedFloors = FMath::Max(0, FMath::RoundToInt(OccupiedHeight / FloorHeight));

	return ActorFloorZ + OccupiedFloors * FloorHeight;
}

double UAsteriaPlacePieceTool::FloorZAtHit(const FHitResult& Hit, double FloorHeight) const
{
	// 여기서는 히트 좌표의 Z를 써도 된다. 수직면 위에서는 커서를 위아래로 움직이면 값이 같이
	// 변하므로, 경계를 넘는 건 의도된 조작이지 진동이 아니다. 윗면(값이 상수)일 때만 위험하다.
	const double HitZ = Hit.ImpactPoint.Z;

	const AActor* HitActor = Hit.GetActor();
	if (HitActor == nullptr)
	{
		const double Ground = static_cast<double>(Settings->GroundPlaneZ);
		return Ground + FMath::GridSnap(HitZ - Ground, FloorHeight);
	}

	// 층 칸은 맞은 피스의 **시각적 바닥**에서부터 센다. 피스 배치 Z나 GroundPlaneZ에서 세면
	// 피벗이 가운데인 피스에서 층 구간이 반 칸 어긋나, 3층 아래쪽을 가리켜도 2층이 나온다.
	// 바닥에서 세면 피벗 위치와 무관하고, 여러 층짜리 피스의 중간 층도 그대로 잡힌다.
	const FBox ActorBounds = HitActor->GetComponentsBoundingBox(/*bNonColliding=*/true);
	const double HeightAboveBottom = HitZ - ActorBounds.Min.Z;
	const int32 FloorIndexInPiece = FMath::Max(0, FMath::FloorToInt32(HeightAboveBottom / FloorHeight));

	return HitActor->GetActorLocation().Z + FloorIndexInPiece * FloorHeight;
}

bool UAsteriaPlacePieceTool::ComputePlacement(const FInputDeviceRay& DeviceRay, FTransform& OutTransform) const
{
	UWorld* World = TargetWorld.Get();
	if (World == nullptr)
	{
		return false;
	}

	const FRay& Ray = DeviceRay.WorldRay;
	const double FloorHeight = FMath::Max(1.0, static_cast<double>(Settings->FloorHeight));

	FVector SurfacePoint;
	double BaseZ;

	FHitResult Hit;
	if (TraceWorld(DeviceRay, Hit))
	{
		SurfacePoint = Hit.ImpactPoint;

		// 옆면을 맞혔을 때 그 면 바깥으로 나가는 방향. 윗면·아랫면이면 길이가 0이다.
		const FVector OutwardDirection = FVector(Hit.ImpactNormal.X, Hit.ImpactNormal.Y, 0.0).GetSafeNormal();

		if (Hit.ImpactNormal.Z >= UpFacingNormalThreshold || OutwardDirection.IsNearlyZero())
		{
			// 윗면(또는 아랫면): 맞은 액터 위에 그대로 얹는다.
			BaseZ = ResolveStackZ(Hit, FloorHeight);
		}
		else
		{
			// 옆면: 커서가 닿은 높이의 층에, 그 벽 바깥 칸으로. 아래에 받쳐줄 게 없어도 놓는다.
			BaseZ = FloorZAtHit(Hit, FloorHeight);

			// 반 칸 밀어내야 옆 칸으로 스냅된다. 히트 지점을 그대로 스냅하면 맞은 블럭과 같은
			// 칸으로 반올림돼 겹친다. 앱실론은 벽면이 격자선 위에 정확히 놓인 경우(피벗이
			// 모서리인 피스) 반올림 타이에 걸려 두 칸을 오가는 걸 막는다.
			SurfacePoint += OutwardDirection * (static_cast<double>(Settings->GridSize) * 0.5 + GridTieBreakEpsilon);
		}
	}
	else
	{
		// 빈 공간 — 0층 평면으로 떨어진다.
		if (FMath::IsNearlyZero(Ray.Direction.Z))
		{
			return false;
		}

		const double Distance = (static_cast<double>(Settings->GroundPlaneZ) - Ray.Origin.Z) / Ray.Direction.Z;
		if (Distance <= 0.0)
		{
			return false;
		}

		SurfacePoint = Ray.PointAt(Distance);
		BaseZ = static_cast<double>(Settings->GroundPlaneZ);
	}

	const double PlacementZ = BaseZ + static_cast<double>(Settings->FloorOffset) * FloorHeight;

	OutTransform = AsteriaBuild::SnapToGrid(SurfacePoint, PlacementZ, Settings->GridSize, Settings->Yaw);
	return true;
}

bool UAsteriaPlacePieceTool::TraceWorld(const FInputDeviceRay& DeviceRay, FHitResult& OutHit) const
{
	UWorld* World = TargetWorld.Get();
	if (World == nullptr)
	{
		return false;
	}

	const FRay& Ray = DeviceRay.WorldRay;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AsteriaPlacePiece), /*bTraceComplex=*/true);
	// 고스트는 충돌이 꺼져 있지만 명시적으로도 뺀다 — 트레이스에 걸리면 자기 위로 기어오른다.
	Params.AddIgnoredActor(PreviewActor);

	return World->LineTraceSingleByChannel(
		OutHit, Ray.Origin, Ray.PointAt(HALF_WORLD_MAX), ECC_Visibility, Params);
}

void UAsteriaPlacePieceTool::RemoveActorUnderCursor(const FInputDeviceRay& DeviceRay)
{
	UWorld* World = TargetWorld.Get();

	FHitResult Hit;
	if (World == nullptr || !TraceWorld(DeviceRay, Hit))
	{
		return;
	}

	AActor* Target = Hit.GetActor();
	if (Target == nullptr || Target == PreviewActor)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("RemovePiece", "Remove Build Piece"));

	// 선택된 채로 지우면 선택 집합에 죽은 참조가 남는다.
	if (Target->IsSelected())
	{
		GEditor->SelectActor(Target, /*bInSelected=*/false, /*bNotify=*/true);
	}

	Target->Modify();
	World->EditorDestroyActor(Target, /*bShouldModifyLevel=*/true);

	RemoveTargetActor = nullptr;
}

void UAsteriaPlacePieceTool::OnUpdateModifierState(int ModifierID, bool bIsOn)
{
	// Shift는 눌림·뗌 양쪽을 다 봐야 배치 모드로 되돌아온다.
	if (ModifierID == RemoveModifierId)
	{
		if (bRemoveModifierDown != bIsOn)
		{
			bRemoveModifierDown = bIsOn;
			if (bHasLastHoverRay)
			{
				OnUpdateHover(LastHoverRay);
			}
		}
		return;
	}

	if (!bIsOn)
	{
		return;
	}

	if (ModifierID == FloorUpModifierId)
	{
		++Settings->FloorOffset;
	}
	else if (ModifierID == FloorDownModifierId)
	{
		--Settings->FloorOffset;
	}
	else
	{
		return;
	}

	// 키로 바꾼 값도 좌측 패널에 반영한다.
	NotifyOfPropertyChangeByTool(Settings);

	// 마우스를 안 움직여도 고스트와 그리드가 바로 새 층으로 올라가야 한다.
	if (bHasHoverSnap)
	{
		OnUpdateHover(LastHoverRay);
	}
}

#undef LOCTEXT_NAMESPACE
