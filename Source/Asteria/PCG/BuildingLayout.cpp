#include "PCG/BuildingLayout.h"

namespace BuildingLayout
{
	namespace
	{
		// 문이 차지하는 첫·끝 칸(X, Y). 가운데 + Offset이고 모서리 칸에 닿으면 멈춘다. 문 없으면 -1.
		// 홀수 칸 벽은 한 칸(1~Cells-2), 짝수 칸 벽은 칸 경계(2~Cells-2) 양옆 두 칸.
		FIntPoint DoorCells(int32 Cells, const FBuildingDoor& Door)
		{
			if (!Door.bEnabled)
			{
				return FIntPoint(-1, -1);
			}
			if (Cells % 2 == 0)
			{
				if (Cells < 4)
				{
					return FIntPoint(-1, -1);
				}
				const int32 K = FMath::Clamp(Cells / 2 + Door.Offset, 2, Cells - 2);
				return FIntPoint(K - 1, K);
			}
			if (Cells < 3)
			{
				return FIntPoint(-1, -1);
			}
			const int32 K = FMath::Clamp(Cells / 2 + Door.Offset, 1, Cells - 2);
			return FIntPoint(K, K);
		}

		// 양 끝에서 센 번호가 홀수인 칸에 창(좌우 대칭).
		bool IsWindowCell(int32 Cells, int32 i)
		{
			return FMath::Min(i, Cells - 1 - i) % 2 == 1;
		}
	}

	FBuilder::FBuilder(int32 InWidth, int32 InHeight, const FBuildingDoor (&Doors)[4])
		: Width(InWidth)
		, Height(InHeight)
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
			W.DoorCells = DoorCells(W.Cells, Doors[Side]);
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
			const FIntPoint Door = W.DoorCells;

			for (int32 i = 0; i < W.Cells; ++i)
			{
				if (i >= Door.X && i <= Door.Y)
				{
					continue;
				}
				AddOnWall(IsWindowCell(W.Cells, i) ? Mesh::WindowWall : Mesh::Wall, Side, i * Cell);
			}
			if (Door.X < 0)
			{
				continue;
			}

			const double DoorStart = Door.X * Cell;
			if (Door.Y == Door.X)
			{
				AddOnWall(Mesh::Entrance, Side, DoorStart);
				continue;
			}
			// 두 칸 문: 반쪽 벽(150) + 문(300, 가운데) + 반쪽 벽(150).
			AddOnWall(Mesh::HalfLeft, Side, DoorStart);
			AddOnWall(Mesh::Entrance, Side, DoorStart + Cell * 0.5);
			AddOnWall(Mesh::HalfRight, Side, DoorStart + Cell * 1.5);
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
