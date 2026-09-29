#pragma once

#include "CoreMinimal.h"
#include "PCGSettings.h"
#include "PCGGuildRoofSettings.generated.h"

class UStaticMesh;

/**
 * PCG_GuildShell 외곽 벽·문, 박공지붕, 실내 천장·꾸밈 배치. 건물 칸 수(ShellWidth·ShellHeight)로 점을 만들고,
 * 점마다 Mesh 속성(메시 경로)을 달아 Static Mesh Spawner(By Attribute) 하나로 뿌린다. 조명 자리는 Lights, 문틀·문짝은 DoorParts 핀으로 따로 낸다.
 * 외곽선은 월드 원점에서 (가로×300, 세로×300)까지. 용마루는 긴 변을 따라가고, 같으면 X 방향.
 */
UCLASS(BlueprintType, ClassGroup = (Procedural))
class ASTERIA_API UPCGGuildRoofSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	UPCGGuildRoofSettings();

#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("GuildRoof")); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCGGuildRoof", "NodeTitle", "Guild Roof"); }
	virtual FText GetNodeTooltipText() const override { return NSLOCTEXT("PCGGuildRoof", "NodeTooltip", "Places guild shell walls, doors, roof, interior ceiling and decor for the given cell counts."); }
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Spatial; }
#endif

	// 가로(X) 칸 수. 그래프의 ShellWidth 파라미터를 덮어쓰기 핀으로 받는다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable, ClampMin = "1"))
	int32 ShellWidth = 8;

	// 세로(Y) 칸 수.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable, ClampMin = "1"))
	int32 ShellHeight = 8;

	// 트러스 간격(칸). 양 끝 박공벽 위치에는 두지 않는다. 0이면 트러스 없음.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable, ClampMin = "0"))
	int32 TrussEvery = 2;

	// 외곽 벽(Tavern_C)과 문. 문 중심 = 벽 중심 + 오프셋×300(짝수 칸 벽은 두 칸 경계, 홀수는 칸 한가운데), 모서리 칸에 닿으면 멈춤.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Walls", meta = (PCG_Overridable))
	bool bWalls = true;

	// 문 위치(그래프 파라미터 DoorNegY 등). 벽·문 배치와, 벽 기둥·벽 등·깃발이 문을 피하는 데 쓴다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Doors", meta = (PCG_Overridable))
	bool bDoorNegY = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Doors", meta = (PCG_Overridable))
	int32 DoorNegYOffset = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Doors", meta = (PCG_Overridable))
	bool bDoorPosX = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Doors", meta = (PCG_Overridable))
	int32 DoorPosXOffset = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Doors", meta = (PCG_Overridable))
	bool bDoorPosY = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Doors", meta = (PCG_Overridable))
	int32 DoorPosYOffset = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Doors", meta = (PCG_Overridable))
	bool bDoorNegX = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Doors", meta = (PCG_Overridable))
	int32 DoorNegXOffset = 0;

	// 실내 천장: 경사 천장은 벽 위 CeilingRise까지만 오르고 가운데는 들보가 드러난 평천장. 짧은 변 3칸 이상일 때.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interior", meta = (PCG_Overridable))
	bool bFlatCeiling = true;

	// 지붕 밑면 시작(벽 윗면+44)에서 평천장까지 높이. 150 단위(경사판 줄)로 맞춘다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interior", meta = (PCG_Overridable, ClampMin = "150"))
	double CeilingRise = 300.0;

	// 트러스 벽 기둥·버팀대, 가로보에 매단 랜턴, 벽 등, 벽 깃발.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interior", meta = (PCG_Overridable))
	bool bInteriorDecor = true;

	// 박공 바탕을 회벽 블록으로. 끄면 판자 블록.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gable", meta = (PCG_Overridable))
	bool bPlasterGable = true;

	// 박공 가운데 창. 아랫단은 짧은 변 3칸 이상일 때 큰 창, 그 위는 여유가 있을 때 좁은 창.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gable", meta = (PCG_Overridable))
	bool bGableWindows = true;

	// 박공 앞뒤 면에 목골조(층마다 가로대, 창 옆 기둥, 가운데 기둥, 45도 버팀대).
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gable", meta = (PCG_Overridable))
	bool bGableTimber = true;

	// 창 위 여유가 있으면 바깥 면 가운데에 깃발.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Gable", meta = (PCG_Overridable))
	bool bGableBanner = true;

	// 벽(300×400, 피벗 아래·왼쪽, +X로 뻗고 두께 -Y) / 문 옆 조각용 반쪽 벽(150).
	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> WallMesh;

	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> WallHalfMesh;

	// 문 위 벽(폭 180) / 문틀 / 문짝 좌·우. 문틀·문짝은 통로를 막지 않게 DoorParts 핀으로 따로 낸다.
	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> DoorCapMesh;

	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> DoorFrameMesh;

	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> DoorLeftMesh;

	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> DoorRightMesh;

	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> SlopeMesh;

	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> SlopeVergeMesh;

	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> EaveMesh;

	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> EaveVergeMesh;

	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> RidgeMesh;

	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> GableSquareMesh;

	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> GableTriMesh;

	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> EaveFillMesh;

	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> GableSquarePlasterMesh;

	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> GableTriPlasterMesh;

	// 300×300 창 벽(벽과 같은 피벗 규칙).
	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> WindowMesh;

	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> WindowThinMesh;

	// 목골조 보(300, 두께 6·높이 21).
	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> TimberMesh;

	// 위 끝 가운데가 피벗, 아래로 약 2m 늘어지는 천. 박공 바깥에 쓴다.
	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> BannerMesh;

	// 트러스 가로보·평천장 테두리 보·등뼈 보(300 단위, 폭 80).
	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> TieBeamMesh;

	// 서까래·기둥·장선·버팀대(300, 20×20).
	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> RafterMesh;

	// 평천장 판재(300×300, 모서리 피벗). Z를 뒤집어 아래를 보게 놓는다.
	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> CeilingPlankMesh;

	// 사슬 끝에 거는 랜턴(바닥이 피벗, 위 끝에 고리).
	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> HangingLanternMesh;

	// 사슬 한 마디(길이 68, 위 끝이 피벗).
	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> ChainMesh;

	// 벽 등(벽면이 피벗, +X로 튀어나옴).
	UPROPERTY(EditAnywhere, Category = "Meshes")
	TSoftObjectPtr<UStaticMesh> SconceMesh;

	// 실내 벽 깃발. 칸마다 돌려 쓴다.
	UPROPERTY(EditAnywhere, Category = "Meshes")
	TArray<TSoftObjectPtr<UStaticMesh>> WallBannerMeshes;

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override { return {}; }
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	virtual FPCGElementPtr CreateElement() const override;
};

class FPCGGuildRoofElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};
