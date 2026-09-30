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
	// 칸 수 상한. 볼륨 범위(0~9600)에 맞춘 값.
	constexpr int32 MaxCells = 32;
	// 모서리 기둥을 벽 두께(40) 가운데에 놓는 외곽선 바깥 거리.
	constexpr double CornerInset = 20.0;

	// 메시. 배치 코드가 각 메시의 피벗·치수에 맞춰져 있어 메시를 바꾸면 해당 배치 코드도 봐야 한다.
	namespace Mesh
	{
		// 바닥 타일(피벗 모서리, +X·-Y로 뻗음)
		inline const FSoftObjectPath Floor(TEXT("/Game/Hearthvale/Meshes/Floor/SM_Floor_Tiles_300.SM_Floor_Tiles_300"));
		// 벽(300×300, 피벗 아래·왼쪽, +X로 뻗고 두께 -Y) / 창 벽(-Y면이 돌, 벽과 같은 yaw로 놓아 돌이 바깥을 봄)
		inline const FSoftObjectPath Wall(TEXT("/Game/Hearthvale/Meshes/Walls/SM_Wall_Tavern_D.SM_Wall_Tavern_D"));
		inline const FSoftObjectPath WindowWall(TEXT("/Game/Hearthvale/Meshes/Walls/SM_Wall_Tavern_D_Window.SM_Wall_Tavern_D_Window"));
		// 문틀 포함 한 칸 문 / 짝수 칸 벽 가운데 문 양옆 반쪽 벽(150폭)
		inline const FSoftObjectPath Entrance(TEXT("/Game/Hearthvale/Meshes/Walls/SM_Wall_Tavern_D_Entrance_300.SM_Wall_Tavern_D_Entrance_300"));
		inline const FSoftObjectPath HalfLeft(TEXT("/Game/Hearthvale/Meshes/Walls/SM_Wall_Tavern_D_Half_lt.SM_Wall_Tavern_D_Half_lt"));
		inline const FSoftObjectPath HalfRight(TEXT("/Game/Hearthvale/Meshes/Walls/SM_Wall_Tavern_D_Half_rt.SM_Wall_Tavern_D_Half_rt"));
		// 모서리 기둥(40×40×300, 중심 피벗). D 세트에 모서리 부품이 없어 이음새를 가린다.
		inline const FSoftObjectPath Corner(TEXT("/Game/Hearthvale/Meshes/Walls/SM_Wall_House_A_Column_B.SM_Wall_House_A_Column_B"));
	}

	/**
	 * 칸 수와 문 설정으로 건물 배치를 계산한다. PCG와 무관한 순수 계산.
	 * 외곽선은 건물 로컬 원점에서 (가로×300, 세로×300)까지.
	 */
	class FBuilder
	{
	public:
		// Doors: 월드 변 순서 NegY, PosX, PosY, NegX.
		FBuilder(int32 InWidth, int32 InHeight, const FBuildingDoor (&Doors)[4]);

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
			FIntPoint DoorCells = FIntPoint(-1, -1); // 문이 차지하는 첫·끝 칸, 문 없으면 -1

			FVector2D Inward() const { return FVector2D(-Dir.Y, Dir.X); }
		};

		int32 Width;
		int32 Height;
		FWall Walls[4];

		void BuildFloor();
		void BuildWalls();
		void BuildCorners();

		// 벽 Side의 시작에서 진행 방향 거리 Along 위치에 벽 yaw로 놓는다.
		void AddOnWall(const FSoftObjectPath& Mesh, int32 Side, double Along);
		void Add(const FSoftObjectPath& Mesh, const FVector2D& Pos, double Yaw);
	};
}
