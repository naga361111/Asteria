#pragma once

#include "CoreMinimal.h"
#include "Building/GuildShellDoor.h"

// 길드 건물 치수. Hearthvale 300 격자·Tavern_C 벽 400 기준.
// 지붕 부품 치수는 Tools/Blender/guild_roof_parts.py와 같아야 한다.
namespace GuildShell
{
	constexpr double Cell = 300.0;
	constexpr double Run = 150.0;          // 경사판 한 줄의 수평(=수직) 길이, 45도
	constexpr double WallTop = 400.0;      // Tavern_C 높이
	constexpr double WallOuter = 44.0;     // Tavern_C 바깥 면(외곽선에서 바깥으로 44)
	constexpr double WallFace = 6.0;       // Tavern_C 실내 면(외곽선에서 안쪽으로 6)
	constexpr double GableBase = WallOuter; // 지붕 밑면은 벽 바깥 윗모서리에서 45도로 오르므로 외곽선 위에서 44 높다
	constexpr double RoofBase = WallTop + GableBase;
	constexpr double CeilingRise = 300.0;   // 지붕 밑면 시작(444)에서 평천장까지. 경사판 줄(150) 배수여야 한다.
	constexpr double BeamHalf = 10.0;      // 20×20 보·80폭 보의 반두께
	constexpr double TieBeamZ = 430.0;     // 트러스 가로보 중심(420~440)

	// 메시. 배치 코드가 각 메시의 피벗·치수에 맞춰져 있어 메시를 바꾸면 해당 배치 코드도 봐야 한다.
	namespace Mesh
	{
		inline const FSoftObjectPath Floor(TEXT("/Game/Hearthvale/Meshes/Floor/SM_Floor_Tiles_300.SM_Floor_Tiles_300"));
		inline const FSoftObjectPath Column(TEXT("/Game/Hearthvale/Meshes/Column/SM_Column_400.SM_Column_400"));
		// 벽(300×400, 피벗 아래·왼쪽, +X로 뻗고 두께 -Y) / 문 옆 조각용 반쪽 벽(150) / Tavern_C 줄에 끼우는 창 벽
		inline const FSoftObjectPath Wall(TEXT("/Game/Hearthvale/Meshes/Walls/SM_Wall_Tavern_C.SM_Wall_Tavern_C"));
		inline const FSoftObjectPath WallHalf(TEXT("/Game/Hearthvale/Meshes/Walls/SM_Wall_Tavern_C_Half.SM_Wall_Tavern_C_Half"));
		inline const FSoftObjectPath WindowWall(TEXT("/Game/Map/Building/Meshes/Walls/SM_GuildWall_Window.SM_GuildWall_Window"));
		// 문 위 벽(폭 180) / 문틀 / 문짝
		inline const FSoftObjectPath DoorCap(TEXT("/Game/Hearthvale/Meshes/Walls/SM_Door_Frame_B_Cap.SM_Door_Frame_B_Cap"));
		inline const FSoftObjectPath DoorFrame(TEXT("/Game/Hearthvale/Meshes/Doors/SM_Door_Frame_A.SM_Door_Frame_A"));
		inline const FSoftObjectPath DoorLeft(TEXT("/Game/Hearthvale/Meshes/Doors/SM_Door_A_LT.SM_Door_A_LT"));
		inline const FSoftObjectPath DoorRight(TEXT("/Game/Hearthvale/Meshes/Doors/SM_Door_A_RT.SM_Door_A_RT"));
		// 지붕 부품(Tools/Blender/guild_roof_parts.py)
		inline const FSoftObjectPath Slope(TEXT("/Game/Map/Building/Meshes/Roof/SM_GuildRoof_Slope.SM_GuildRoof_Slope"));
		inline const FSoftObjectPath SlopeVerge(TEXT("/Game/Map/Building/Meshes/Roof/SM_GuildRoof_SlopeVerge.SM_GuildRoof_SlopeVerge"));
		inline const FSoftObjectPath Eave(TEXT("/Game/Map/Building/Meshes/Roof/SM_GuildRoof_Eave.SM_GuildRoof_Eave"));
		inline const FSoftObjectPath EaveVerge(TEXT("/Game/Map/Building/Meshes/Roof/SM_GuildRoof_EaveVerge.SM_GuildRoof_EaveVerge"));
		inline const FSoftObjectPath EaveFill(TEXT("/Game/Map/Building/Meshes/Roof/SM_GuildRoof_EaveFill.SM_GuildRoof_EaveFill"));
		inline const FSoftObjectPath Ridge(TEXT("/Game/Map/Building/Meshes/Roof/SM_GuildRoof_Ridge.SM_GuildRoof_Ridge"));
		inline const FSoftObjectPath GableBand(TEXT("/Game/Map/Building/Meshes/Roof/SM_GuildRoof_GableSquare.SM_GuildRoof_GableSquare"));
		inline const FSoftObjectPath GableSquare(TEXT("/Game/Map/Building/Meshes/Roof/SM_GuildRoof_GableSquare_Plaster.SM_GuildRoof_GableSquare_Plaster"));
		inline const FSoftObjectPath GableTri(TEXT("/Game/Map/Building/Meshes/Roof/SM_GuildRoof_GableTri_Plaster.SM_GuildRoof_GableTri_Plaster"));
		// 박공 창(300×300, 벽과 같은 피벗 규칙)
		inline const FSoftObjectPath GableWindow(TEXT("/Game/Hearthvale/Meshes/Walls/SM_Wall_Tavern_D_Window.SM_Wall_Tavern_D_Window"));
		inline const FSoftObjectPath GableWindowThin(TEXT("/Game/Hearthvale/Meshes/Walls/SM_Wall_Tavern_D_Window_Thin.SM_Wall_Tavern_D_Window_Thin"));
		// 목골조 보(300, 두께 6·높이 21) / 박공 깃발(위 끝 가운데가 피벗, 아래로 약 2m)
		inline const FSoftObjectPath Timber(TEXT("/Game/Hearthvale/Meshes/Beams/SM_WoodBeam_A.SM_WoodBeam_A"));
		inline const FSoftObjectPath GableBanner(TEXT("/Game/Hearthvale/Meshes/Fabric/SM_Banner_c.SM_Banner_c"));
		// 트러스 가로보·평천장 보(300, 폭 80) / 서까래·기둥·장선·버팀대·띠보(300, 20×20, 한쪽 끝 피벗)
		inline const FSoftObjectPath TieBeam(TEXT("/Game/Hearthvale/Meshes/Beams/sm_beam_wide_300.sm_beam_wide_300"));
		inline const FSoftObjectPath Rafter(TEXT("/Game/Hearthvale/Meshes/Beams/SM_WoodBeam_B.SM_WoodBeam_B"));
		// 평천장 판재(300×300, 모서리 피벗)
		inline const FSoftObjectPath CeilingPlank(TEXT("/Game/Hearthvale/Meshes/Floor/SM_PlanksFloor_A.SM_PlanksFloor_A"));
		// 매단 랜턴(바닥 피벗) / 사슬 한 마디(길이 68, 위 끝 피벗) / 벽 등(벽면 피벗, +X로 튀어나옴)
		inline const FSoftObjectPath HangingLantern(TEXT("/Game/Hearthvale/Meshes/Details/SM_Lantern_c.SM_Lantern_c"));
		inline const FSoftObjectPath Chain(TEXT("/Game/Hearthvale/Meshes/Chain/SM_Chain_Line_a.SM_Chain_Line_a"));
		inline const FSoftObjectPath Sconce(TEXT("/Game/Hearthvale/Meshes/Lamp/SM_Latern_C.SM_Latern_C"));
		// 실내 벽 깃발. 칸마다 돌려 쓴다.
		inline const FSoftObjectPath WallBanners[] = {
			FSoftObjectPath(TEXT("/Game/Hearthvale/Meshes/Fabric/SM_Banner_a.SM_Banner_a")),
			FSoftObjectPath(TEXT("/Game/Hearthvale/Meshes/Fabric/SM_Banner_d.SM_Banner_d")),
			FSoftObjectPath(TEXT("/Game/Hearthvale/Meshes/Fabric/SM_Banner_g.SM_Banner_g")),
			FSoftObjectPath(TEXT("/Game/Hearthvale/Meshes/Fabric/SM_Banner_e.SM_Banner_e")),
		};
	}

	// 점마다 메시 경로를 단 배치 목록.
	struct FGuildShellMeshPoints
	{
		TArray<FTransform> Transforms;
		TArray<FSoftObjectPath> Meshes;
	};

	/**
	 * 칸 수와 문 설정으로 길드 건물 배치를 계산한다. PCG와 무관한 순수 계산.
	 *
	 * 좌표: 건물 기준 좌표(U=용마루 방향, V=경사 방향)에서 계산하고, 세로가 더 길면 90도 돌려 월드에 놓는다.
	 * 외곽선은 월드 원점에서 (가로×300, 세로×300)까지. 용마루는 긴 변을 따라가고, 같으면 X 방향.
	 *
	 * 기능별 구현 파일:
	 *   GuildShellBuilder.cpp  — 좌표 틀, 바닥·모서리 기둥·띠보, 외곽 벽·창 벽·문
	 *   GuildShellRoof.cpp     — 지붕, 박공벽
	 *   GuildShellInterior.cpp — 평천장·트러스, 장식·조명
	 */
	class FGuildShellBuilder
	{
	public:
		// Doors: 월드 변 순서 NegY, PosX, PosY, NegX.
		FGuildShellBuilder(int32 InWidth, int32 InHeight, const FGuildShellDoor (&Doors)[4]);

		FGuildShellMeshPoints Blocking;    // 충돌 있음
		FGuildShellMeshPoints NoCollision; // 바닥·모서리 기둥·띠보·문틀·문짝(통로를 막지 않게)
		TArray<FTransform> Lights;         // 매단 랜턴 조명 자리
		TArray<FTransform> SconceLights;   // 벽 등 조명 자리

	private:
		// 외곽 벽 하나. 위에서 볼 때 시계 방향으로 돌고, 실내는 진행 방향 기준 로컬 +Y 쪽이다.
		// 0 = A면(V=0), 1 = U=Len 박공, 2 = B면(V=Span), 3 = U=0 박공.
		struct FWall
		{
			FVector2D Start;
			FVector2D Dir;
			double Yaw = 0.0;
			int32 Cells = 0;
			double DoorAlong = -1.0;        // 문 중심(벽 시작에서 잰 거리), 문 없으면 음수
			TArray<FVector2D> Openings;     // 문·창 구간(벽 시작에서 잰 거리). 생성자에서 채우고 장식이 피한다.

			FVector2D Inward() const { return FVector2D(-Dir.Y, Dir.X); }
		};

		int32 Width;
		int32 Height;
		bool bSwap;     // 세로가 더 길어 90도 돌려 놓는지
		int32 L;        // 긴 변 칸 수(용마루 방향)
		int32 S;        // 짧은 변 칸 수(경사 방향)
		double Len;
		double Span;
		double Half;    // 용마루까지 수평 거리
		bool bFlat;     // 평천장인지(짧은 변 3칸 이상). 평천장 위는 실내에서 안 보여 짓지 않는다.
		int32 RoofRows; // 짓는 경사판 줄 수: 평천장이면 천장 높이까지, 아니면 용마루까지
		FWall Walls[4];

		void BuildBase();
		void BuildWalls();
		void BuildRoof();
		void BuildGables();
		void BuildCeiling(TArray<double>& OutTrussU);
		void BuildDecor(const TArray<double>& TrussU);

		FVector ToWorld(double U, double V, double Z) const;
		void Add(FGuildShellMeshPoints& Out, const FSoftObjectPath& Mesh, double U, double V, double Z, double Yaw,
			FVector Scale = FVector::OneVector, double Pitch = 0.0);
		// Blocking에 기본 치수 300인 보를 Length 길이로 늘려 놓는다.
		void Beam(const FSoftObjectPath& Mesh, double U, double V, double Z, double Yaw, double Pitch, double Length);
		// 벽 Side를 따라: Along = 벽 시작에서 진행 방향 거리, Lateral = 실내 쪽 거리. 건물 좌표(U, V)로 바꾼다.
		FVector2D OnWall(int32 Side, double Along, double Lateral) const;
		// OnWall의 역: 건물 좌표(U, V)를 벽 Side 시작에서 잰 진행 방향 거리로.
		double AlongWall(int32 Side, double U, double V) const;
		void AddOnWall(FGuildShellMeshPoints& Out, const FSoftObjectPath& Mesh, int32 Side, double Along, double Lateral, double Z,
			double YawOffset = 0.0, FVector Scale = FVector::OneVector, double Pitch = 0.0);
		void AddLight(TArray<FTransform>& Out, double U, double V, double Z) const;
		// 벽 Side의 Along 위치(±HalfWidth)가 문·창 구간과 겹치는지.
		bool IsOpen(int32 Side, double Along, double HalfWidth) const;
	};
}
