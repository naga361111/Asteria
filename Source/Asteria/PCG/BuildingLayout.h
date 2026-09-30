#pragma once

#include "CoreMinimal.h"
#include "BuildingLayout.generated.h"

// PCG_Building 한 변의 문 설정. 그래프 파라미터(DoorNegY/PosX/PosY/NegX) 타입으로 쓰여 디테일 패널에서 변마다 묶어 편집한다.
USTRUCT(BlueprintType)
struct FBuildingDoor
{
	GENERATED_BODY()

	// 이 변에 문을 둘지.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door")
	bool bEnabled = false;

	// 가운데 칸에서 옮길 칸 수. 양수=건물 안에서 벽을 볼 때 오른쪽, 음수=왼쪽. 모서리 칸에 닿으면 멈춘다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door")
	int32 Offset = 0;
};

// 건물(바닥·외곽 벽) 배치. Hearthvale 300 격자·Tavern_D 벽 300 기준.
namespace BuildingLayout
{
	constexpr double Cell = 300.0;
	// 가운데 칸을 뺀 한쪽 칸 수 상한. 칸 수 31(9300)이 볼륨 범위(0~9600) 안에 들어가는 값.
	constexpr int32 MaxHalfCells = 15;
	// 모서리 기둥을 벽 두께(40) 가운데에 놓는 외곽선 바깥 거리.
	constexpr double CornerInset = 20.0;
	// _D 벽 높이. 판자 줄이 이 높이에서 시작한다.
	constexpr double WallHeight = 300.0;
	// 판자 벽 높이. 위쪽 모서리 기둥(300)을 이 높이로 줄인다.
	constexpr double PlankHeight = 160.0;
	// 벽기둥 두께(25.2)의 절반. 외곽선에서 벽기둥 중심까지 거리라 뒷면이 외곽선에 맞아 벽 두께 속에 묻힌다.
	constexpr double PilasterHalfDepth = 6.0;
	// 벽기둥 폭(59.8)의 절반. 벽 끝에서 이만큼 들여 놓아 모서리 벽기둥의 가장자리를 모서리에 맞춘다.
	constexpr double PilasterHalfWidth = 30.0;

	// 메시. 배치 코드가 각 메시의 피벗·치수에 맞춰져 있어 메시를 바꾸면 해당 배치 코드도 봐야 한다.
	namespace Mesh
	{
		// 판자 바닥(300×300, 피벗 모서리, +X·-Y로 뻗음)
		inline const FSoftObjectPath Floor(TEXT("/Game/Hearthvale/Meshes/Floor/SM_PlanksFloor_A.SM_PlanksFloor_A"));
		// 벽(300×300, 피벗 아래·왼쪽, +X로 뻗고 두께 -Y) / 창 벽(-Y면이 돌, 벽과 같은 yaw로 놓아 돌이 바깥을 봄)
		inline const FSoftObjectPath Wall(TEXT("/Game/Hearthvale/Meshes/Walls/SM_Wall_Tavern_D.SM_Wall_Tavern_D"));
		inline const FSoftObjectPath WindowWall(TEXT("/Game/Hearthvale/Meshes/Walls/SM_Wall_Tavern_D_Window.SM_Wall_Tavern_D_Window"));
		// 문틀 포함 한 칸 문
		inline const FSoftObjectPath Entrance(TEXT("/Game/Hearthvale/Meshes/Walls/SM_Wall_Tavern_D_Entrance_300.SM_Wall_Tavern_D_Entrance_300"));
		// 모서리 기둥(40×40×300, 중심 피벗). D 세트에 모서리 부품이 없어 이음새를 가린다.
		inline const FSoftObjectPath Corner(TEXT("/Game/Hearthvale/Meshes/Walls/SM_Wall_House_A_Column_B.SM_Wall_House_A_Column_B"));
		// 벽 위 판자 줄(300×160, 벽과 같은 피벗 규칙: 아래·왼쪽, +X로 뻗고 두께 -Y)
		inline const FSoftObjectPath Plank(TEXT("/Game/Hearthvale/Meshes/Walls/SM_Wall_Wooden_Planks_A_300.SM_Wall_Wooden_Planks_A_300"));
		// 벽면에 붙는 납작한 벽기둥(폭 60 × 두께 25 × 높이 305, 아래 가운데 피벗, 폭이 로컬 Y)
		inline const FSoftObjectPath Pilaster(TEXT("/Game/Hearthvale/Meshes/Walls/SM_Wall_House_A_Column.SM_Wall_House_A_Column"));
	}

	/**
	 * 한쪽 칸 수와 문 설정으로 건물 배치를 계산한다. PCG와 무관한 순수 계산.
	 * 칸 수는 가운데 칸 + 양쪽 = 2N+1로 항상 홀수. 외곽선은 건물 로컬 원점에서 (가로 칸 수×300, 세로 칸 수×300)까지.
	 */
	class FBuilder
	{
	public:
		// HalfWidth·HalfHeight: 가운데 칸을 뺀 한쪽 칸 수. Doors: 월드 변 순서 NegY, PosX, PosY, NegX.
		FBuilder(int32 HalfWidth, int32 HalfHeight, const FBuildingDoor (&Doors)[4]);

		// 점마다 트랜스폼과 메시 경로. 건물 로컬 좌표.
		TArray<FTransform> Transforms;
		TArray<FSoftObjectPath> Meshes;

	private:
		// 외곽 벽 하나. 위에서 볼 때 시계 방향으로 돌고, 실내는 진행 방향 기준 로컬 +Y 쪽이다.
		// 0 = NegY(yaw 0), 1 = PosX(90), 2 = PosY(180), 3 = NegX(-90).
		struct FWall
		{
			FVector2D Start;
			FVector2D Dir;
			double Yaw = 0.0;
			int32 Cells = 0;
			int32 DoorCell = -1; // 문 칸 번호, 문 없으면 -1

			FVector2D Inward() const { return FVector2D(-Dir.Y, Dir.X); }
		};

		// 칸 수(2N+1)
		int32 Width;
		int32 Height;
		FWall Walls[4];

		void BuildFloor();
		void BuildWalls();
		void BuildCorners();

		// 벽 Side의 시작에서 진행 방향 거리 Along, 실내 쪽 거리 Lateral, 높이 Z 위치에 벽 yaw + YawOffset으로 놓는다.
		void AddOnWall(const FSoftObjectPath& Mesh, int32 Side, double Along, double Z = 0.0, double Lateral = 0.0, double YawOffset = 0.0);
		void Add(const FSoftObjectPath& Mesh, const FVector2D& Pos, double Yaw, double Z = 0.0, const FVector& Scale = FVector::OneVector);
	};
}
