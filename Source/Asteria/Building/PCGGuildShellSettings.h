#pragma once

#include "CoreMinimal.h"
#include "PCGSettings.h"
#include "PCGGuildShellSettings.generated.h"

/**
 * PCG_GuildShell의 건물 생성 노드 "Guild Shell". 칸 수·문 설정(그래프 파라미터에서 덮어쓰기 핀으로 받음)으로
 * 바닥·벽·문·지붕·박공·천장·장식을 배치한다. 배치 계산은 FGuildShellBuilder(GuildShellBuilder.h).
 * 출력 핀: Out(충돌 있음), NoCollision(바닥·모서리 기둥·띠보·문틀·문짝), Lights(매단 랜턴), SconceLights(벽 등).
 * 메시 점에는 Mesh 속성(메시 경로)이 달려 Static Mesh Spawner(By Attribute)로 뿌린다.
 */
UCLASS(BlueprintType, ClassGroup = (Procedural))
class ASTERIA_API UPCGGuildShellSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	// 칸 수 상한. GuildShellVolume 범위(0~9600)에 맞춘 값. GuildShellService도 이 값을 쓴다.
	static constexpr int32 MaxCells = 32;

#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("GuildShell")); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCGGuildShell", "NodeTitle", "Guild Shell"); }
	virtual FText GetNodeTooltipText() const override { return NSLOCTEXT("PCGGuildShell", "NodeTooltip", "Places the guild building (floor, walls, doors, roof, ceiling, decor) for the given cell counts."); }
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Spatial; }
#endif

	// 가로(X)·세로(Y) 칸 수(한 칸 300). 그래프의 ShellWidth·ShellHeight 파라미터를 받는다. 1~MaxCells로 자른다.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable, ClampMin = "1"))
	int32 ShellWidth = 8;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable, ClampMin = "1"))
	int32 ShellHeight = 8;

	// 변마다 문(그래프 파라미터 DoorNegY 등의 bEnabled·Offset). 문 중심 = 벽 중심 + 오프셋×300, 모서리 칸에 닿으면 멈춘다.
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

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override { return {}; }
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
	virtual FPCGElementPtr CreateElement() const override;
};

class FPCGGuildShellElement : public IPCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};
