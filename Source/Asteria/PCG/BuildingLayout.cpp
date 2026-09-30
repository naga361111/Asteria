#include "PCG/BuildingLayout.h"

namespace BuildingLayout
{
	namespace
	{
		// 문 칸 번호. 가운데 칸 + Offset이고 모서리 칸에 닿으면 멈춘다(1~Cells-2). 3칸 미만 벽이거나 문 없으면 -1.
		int32 DoorCells(int32 Cells, const FBuildingDoor& Door)
		{
			if (!Door.bEnabled || Cells < 3)
			{
				return -1;
			}
			return FMath::Clamp(Cells / 2 + Door.Offset, 1, Cells - 2);
		}

		// 양 끝에서 센 번호가 홀수인 칸에 창(좌우 대칭).
		bool IsWindowCell(int32 Cells, int32 i)
		{
			return FMath::Min(i, Cells - 1 - i) % 2 == 1;
		}
	}

	FBuilder::FBuilder(int32 HalfWidth, int32 HalfHeight, const FBuildingDoor (&Doors)[4])
		: Width(HalfWidth * 2 + 1)
		, Height(HalfHeight * 2 + 1)
	{
		const double Len = Width * Cell;
		const double Span = Height * Cell;
		const FVector2D Corners[4] = { FVector2D(0.0, 0.0), FVector2D(Len, 0.0), FVector2D(Len, Span), FVector2D(0.0, Span) };
		const double Yaws[4] = { 0.0, 90.0, 180.0, -90.0 };
		for (int32 Side = 0; Side < 4; ++Side)
		{
			FWall& W = Walls[Side];
			W.Start = Corners[Side];
			W.Dir = (Corners[(Side + 1) % 4] - Corners[Side]).GetSafeNormal();
			W.Yaw = Yaws[Side];
			W.Cells = Side % 2 == 0 ? Width : Height;
			W.DoorCell = DoorCells(W.Cells, Doors[Side]);
		}

		BuildFloor();
		BuildWalls();
		BuildCorners();
	}

	void FBuilder::Add(const FSoftObjectPath& Mesh, const FVector2D& Pos, double Yaw)
	{
		Transforms.Emplace(FRotator(0.0, Yaw, 0.0), FVector(Pos.X, Pos.Y, 0.0));
		Meshes.Add(Mesh);
	}

	void FBuilder::AddOnWall(const FSoftObjectPath& Mesh, int32 Side, double Along)
	{
		const FWall& W = Walls[Side];
		Add(Mesh, W.Start + W.Dir * Along, W.Yaw);
	}

	void FBuilder::BuildFloor()
	{
		// 바닥 타일: 피벗이 모서리이고 +X·-Y로 뻗는다. 무늬 방향이 돌지 않게 월드 축 기준으로 놓는다.
		for (int32 X = 0; X < Width; ++X)
		{
			for (int32 Y = 0; Y < Height; ++Y)
			{
				Add(Mesh::Floor, FVector2D(X * Cell, (Y + 1) * Cell), 0.0);
			}
		}
	}

	void FBuilder::BuildWalls()
	{
		// 모든 조각을 벽 yaw 그대로 놓는다(돌 면·두께가 바깥).
		for (int32 Side = 0; Side < 4; ++Side)
		{
			const FWall& W = Walls[Side];
			for (int32 i = 0; i < W.Cells; ++i)
			{
				if (i == W.DoorCell)
				{
					AddOnWall(Mesh::Entrance, Side, i * Cell);
					continue;
				}
				AddOnWall(IsWindowCell(W.Cells, i) ? Mesh::WindowWall : Mesh::Wall, Side, i * Cell);
			}
		}
	}

	void FBuilder::BuildCorners()
	{
		for (int32 Side = 0; Side < 4; ++Side)
		{
			const FWall& W = Walls[Side];
			// 이 벽과 앞 벽의 두께 가운데선이 만나는 점.
			const FVector2D Corner = W.Start - (W.Inward() + Walls[(Side + 3) % 4].Inward()) * CornerInset;
			Add(Mesh::Corner, Corner, W.Yaw);
		}
	}
}
