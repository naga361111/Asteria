// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractiveTool.h"
#include "InteractiveToolBuilder.h"
#include "BaseBehaviors/BehaviorTargetInterfaces.h"

#include "AsteriaPlacePieceTool.generated.h"

struct FHitResult;

class AActor;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * 배치 툴의 조정 가능한 값들. UEdMode 툴킷이 활성 툴의 프로퍼티 셋을 좌측 패널 디테일 뷰에
 * 그대로 띄우므로, 여기 UPROPERTY를 추가하는 것이 곧 패널에 노출하는 것이다.
 */
UCLASS()
class UAsteriaPlacePieceToolProperties : public UInteractiveToolPropertySet
{
	GENERATED_BODY()

public:
	/** 배치할 메시. 카탈로그(DataAsset/DataTable)를 정하기 전까지는 이 한 칸이 팔레트 역할을 한다. */
	UPROPERTY(EditAnywhere, Category = "Piece")
	TObjectPtr<UStaticMesh> Piece;

	/** XY 평면 스냅 간격. Hearthvale 키트 기준 300. */
	UPROPERTY(EditAnywhere, Category = "Snap", meta = (ClampMin = "1.0", UIMin = "50.0", UIMax = "1000.0", Delta = "50.0"))
	float GridSize = 300.f;

	/** 층 간격(층고). CurrentFloor 한 칸이 이만큼이다. */
	UPROPERTY(EditAnywhere, Category = "Snap", meta = (ClampMin = "1.0", UIMin = "50.0", UIMax = "1000.0", Delta = "50.0"))
	float FloorHeight = 300.f;

	/** 배치할 때 적용할 Yaw. 회전 단축키는 아직 없고, 값으로만 돌린다. */
	UPROPERTY(EditAnywhere, Category = "Placement", meta = (ClampMin = "-360.0", ClampMax = "360.0", Delta = "90.0"))
	float Yaw = 0.f;

	/**
	 * 자동으로 잡힌 층에서 몇 칸 더 올리거나 내릴지. 평소엔 0이고, 허공이나 한 칸 위에
	 * 억지로 놓아야 할 때만 쓴다. 뷰포트에서 PageUp/PageDown.
	 */
	UPROPERTY(EditAnywhere, Category = "Placement", meta = (ClampMin = "-64", ClampMax = "64"))
	int32 FloorOffset = 0;

	/** 아무것도 안 맞혔을 때 쓰는 0층 높이. */
	UPROPERTY(EditAnywhere, Category = "Placement", meta = (Delta = "100.0"))
	float GroundPlaneZ = 0.f;

	/** 고스트에 덮어씌울 머티리얼. 비우면 메시 원본 머티리얼이 그대로 보인다. */
	UPROPERTY(EditAnywhere, Category = "Preview")
	TObjectPtr<UMaterialInterface> PreviewMaterial;

	/** 고스트 색. PreviewMaterial의 "Color" 파라미터로 들어간다. 알파가 불투명도다. */
	UPROPERTY(EditAnywhere, Category = "Preview")
	FLinearColor PreviewColor = FLinearColor(0.15f, 0.6f, 1.f, 0.35f);

	/** 커서 주변에 현재 GridSize 격자를 그린다. */
	UPROPERTY(EditAnywhere, Category = "Preview")
	bool bDrawGrid = true;

	/** 커서 칸에서 바깥으로 몇 칸까지 그릴지. */
	UPROPERTY(EditAnywhere, Category = "Preview", meta = (ClampMin = "1", ClampMax = "32", EditCondition = "bDrawGrid"))
	int32 GridDrawRadius = 6;

	UPROPERTY(EditAnywhere, Category = "Preview", meta = (EditCondition = "bDrawGrid"))
	FLinearColor GridColor = FLinearColor(0.35f, 0.8f, 1.f, 0.5f);

	/** Shift를 누르고 있을 때 지워질 액터를 감싸는 박스 색. */
	UPROPERTY(EditAnywhere, Category = "Preview")
	FLinearColor RemoveHighlightColor = FLinearColor(1.f, 0.15f, 0.1f, 1.f);

	UAsteriaPlacePieceToolProperties();
};

/**
 * 커서를 따라다니는 고스트 + 클릭 한 번에 그리드 스냅된 StaticMeshActor 하나.
 * 빌드 모드 5요소 중 (2)프리뷰 + (3)스냅 + (5)배치를 담당한다. 회전 입력·검증·팔레트는 아직 없다.
 *
 * **Z 결정 규칙:** 커서가 맞힌 것 위에 알아서 쌓는다. 단 Z는 히트 좌표(ImpactPoint.Z)가 아니라
 * **맞은 액터의 배치 Z**에서 뽑는다 — 히트 좌표는 평평한 면 위에서 상수라, 그 값이 반올림 경계에
 * 앉으면(피벗이 가운데인 피스가 정확히 그렇다) 커서를 안 움직여도 층이 위아래로 뒤집힌다.
 * 액터 배치 Z는 액터당 상수라 그 진동이 구조적으로 불가능하다.
 *
 * **제거:** Shift를 누르면 배치 대신 제거가 된다 — Foliage 페인터의 Shift+클릭 지우기와 같은
 * 관례다. 고스트 대신 커서가 가리킨 액터에 빨간 박스가 씌워지고, 클릭하면 그 액터가 지워진다.
 * 별도 툴로 나누지 않은 건 팔레트 커맨드가 필요해질 뿐 얻는 게 없어서다.
 *
 * 고스트와 실제 배치는 반드시 AsteriaBuild::SnapToGrid 하나만 통과한다 — 둘이 다른 트랜스폼을
 * 계산하기 시작하면 프리뷰가 거짓말을 하게 된다.
 *
 * 고스트는 RF_Transient 임시 에디터 액터라 레벨에 저장되지 않는다.
 *
 * 배치는 FScopedTransaction으로 묶여 있어 Ctrl+Z로 되돌아간다.
 */
UCLASS()
class UAsteriaPlacePieceTool : public UInteractiveTool, public IClickBehaviorTarget, public IHoverBehaviorTarget
{
	GENERATED_BODY()

public:
	void SetTargetWorld(UWorld* InWorld);

	// UInteractiveTool interface
	virtual void Setup() override;
	virtual void Shutdown(EToolShutdownType ShutdownType) override;
	virtual void OnPropertyModified(UObject* PropertySet, FProperty* Property) override;
	virtual void Render(IToolsContextRenderAPI* RenderAPI) override;

	// IClickBehaviorTarget interface
	virtual FInputRayHit IsHitByClick(const FInputDeviceRay& ClickPos) override;
	virtual void OnClicked(const FInputDeviceRay& ClickPos) override;

	// IHoverBehaviorTarget interface
	virtual FInputRayHit BeginHoverSequenceHitTest(const FInputDeviceRay& PressPos) override;
	virtual void OnBeginHover(const FInputDeviceRay& DevicePos) override;
	virtual bool OnUpdateHover(const FInputDeviceRay& DevicePos) override;
	virtual void OnEndHover() override;

	// IModifierToggleBehaviorTarget interface (PageUp/PageDown 층 이동)
	virtual void OnUpdateModifierState(int ModifierID, bool bIsOn) override;

private:
	/** 윗면을 맞혔을 때: 맞은 액터가 차지한 층 수만큼 위. 히트 좌표는 쓰지 않는다. */
	double ResolveStackZ(const FHitResult& Hit, double FloorHeight) const;

	/** 옆면을 맞혔을 때: 커서가 닿은 지점이 그 피스의 몇 번째 층 칸인지 세서 그 높이를 낸다. */
	double FloorZAtHit(const FHitResult& Hit, double FloorHeight) const;

	/** 커서 레이 → 스냅된 배치 트랜스폼. 고스트와 실제 배치가 같이 쓰는 유일한 경로다. */
	bool ComputePlacement(const FInputDeviceRay& DeviceRay, FTransform& OutTransform) const;

	/** 커서 레이로 월드를 쏜다. 고스트는 제외된다. */
	bool TraceWorld(const FInputDeviceRay& DeviceRay, FHitResult& OutHit) const;

	/** 커서가 가리킨 액터를 지운다(트랜잭션으로 묶여 Ctrl+Z로 돌아온다). */
	void RemoveActorUnderCursor(const FInputDeviceRay& DeviceRay);

	void CreatePreview();
	void DestroyPreview();

	/** 고스트의 메시·머티리얼을 현재 설정에 맞춘다. 패널 값이 바뀔 때만 호출한다. */
	void RefreshPreviewAppearance();

	UPROPERTY()
	TObjectPtr<UAsteriaPlacePieceToolProperties> Settings;

	UPROPERTY()
	TObjectPtr<AActor> PreviewActor;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> PreviewComponent;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PreviewMaterialInstance;

	TWeakObjectPtr<UWorld> TargetWorld;

	/** 마지막 호버에서 나온 스냅 위치. 고스트와 그리드가 같은 값을 본다. */
	FVector HoverSnapLocation = FVector::ZeroVector;
	bool bHasHoverSnap = false;

	/** 마우스를 안 움직여도 층 변경·모드 전환을 즉시 반영하려면 마지막 레이가 필요하다. */
	FInputDeviceRay LastHoverRay;
	bool bHasLastHoverRay = false;

	/** Shift 상태. 눌려 있으면 배치 대신 제거다. */
	bool bRemoveModifierDown = false;

	/** 제거 모드에서 지금 커서가 가리키고 있는 액터. */
	TWeakObjectPtr<AActor> RemoveTargetActor;
};

UCLASS()
class UAsteriaPlacePieceToolBuilder : public UInteractiveToolBuilder
{
	GENERATED_BODY()

public:
	virtual bool CanBuildTool(const FToolBuilderState& SceneState) const override;
	virtual UInteractiveTool* BuildTool(const FToolBuilderState& SceneState) const override;
};
