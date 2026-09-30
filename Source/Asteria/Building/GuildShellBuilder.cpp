#include "Building/GuildShellBuilder.h"

namespace GuildShell
{
	namespace
	{
		constexpr double DoorHalfWidth = 90.0;   // 문 위 벽(Door_Frame_B_Cap) 폭 180의 절반
		constexpr double DoorClearance = 40.0;   // 장식은 문 양옆으로 이만큼 더 비운다
		constexpr double SeamOverlap = 5.0;      // 문 옆 조각이 문 쪽으로 겹치는 폭(끝 모서리가 문틀 뒤로 숨음)
		constexpr double WindowHalfWidth = 70.0; // 창 벽의 창틀 반폭(56) + 여유. 장식이 이 구간을 피한다
		constexpr double WallPlateZ = 390.0;     // 벽 윗단 안쪽 띠보 중심(380~400)
		constexpr double CornerInset = (WallOuter - WallFace) * 0.5; // 모서리 기둥: 벽 두께 가운데(외곽선 바깥 19)
		// Column_400(435.6)을 381까지(띠보 밑면에 1 겹치게), 굵기 1.08배. 실내 모서리 틈 방지.
		const FVector ColumnScale(1.08, 1.08, 0.8747);

		// 벽 시작에서 잰 문 중심. 벽 중심 + 오프셋×300이고 모서리 칸에는 닿지 않게 멈춘다.
		// 짝수 칸 벽은 칸 경계(300k, k=1..n-1), 홀수 칸 벽은 칸 한가운데(300k+150, k=1..n-2).
		double DoorCenter(int32 Cells, int32 Offset)
		{
			if (Cells % 2 == 0)
			{
				return FMath::Clamp(Cells / 2 + Offset, 1, Cells - 1) * Cell;
			}
			const int32 K = Cells >= 3 ? FMath::Clamp(Cells / 2 + Offset, 1, Cells - 2) : 0;
			return K * Cell + Cell * 0.5;
		}

		// 문이 차지하는 첫·끝 칸(X, Y). 짝수 칸 벽이면 경계 양옆 두 칸, 홀수면 한 칸. 문 없으면 -1.
		FIntPoint DoorCells(double DoorAlong)
		{
			if (DoorAlong < 0.0)
			{
				return FIntPoint(-1, -1);
			}
			if (FMath::IsNearlyZero(FMath::Fmod(DoorAlong, Cell)))
			{
				const int32 First = FMath::RoundToInt(DoorAlong / Cell) - 1;
				return FIntPoint(First, First + 1);
			}
			const int32 First = FMath::FloorToInt(DoorAlong / Cell);
			return FIntPoint(First, First);
		}

		// 양 끝에서 센 번호가 홀수인 칸에 창(좌우 대칭).
		bool IsWindowCell(int32 Cells, int32 i)
		{
			return FMath::Min(i, Cells - 1 - i) % 2 == 1;
		}
	}

	FGuildShellBuilder::FGuildShellBuilder(int32 InWidth, int32 InHeight, const FGuildShellDoor (&Doors)[4])
		: Width(InWidth)
		, Height(InHeight)
		, bSwap(InWidth < InHeight)
		, L(FMath::Max(InWidth, InHeight))
		, S(FMath::Min(InWidth, InHeight))
		, Len(L * Cell)
		, Span(S * Cell)
		, Half(S * Run)
		, bFlat(Span - 2.0 * CeilingRise >= Cell)
		, RoofRows(bFlat ? FMath::RoundToInt32(CeilingRise / Run) : S)
	{
		const FVector2D Corners[4] = { FVector2D(0.0, 0.0), FVector2D(Len, 0.0), FVector2D(Len, Span), FVector2D(0.0, Span) };
		const double Yaws[4] = { 0.0, 90.0, 180.0, -90.0 };
		for (int32 Side = 0; Side < 4; ++Side)
		{
			FWall& W = Walls[Side];
			W.Start = Corners[Side];
			W.Dir = (Corners[(Side + 1) % 4] - Corners[Side]).GetSafeNormal();
			W.Yaw = Yaws[Side];
			W.Cells = Side % 2 == 0 ? L : S;
			// 90도 돌려 놓으면 월드 변(NegY, PosX, PosY, NegX)이 벽 번호와 하나씩 어긋난다.
			const FGuildShellDoor& Door = Doors[(Side + (bSwap ? 1 : 0)) % 4];
			if (Door.bEnabled)
			{
				W.DoorAlong = DoorCenter(W.Cells, Door.Offset);
			}

			const FIntPoint DoorCell = DoorCells(W.DoorAlong);
			for (int32 i = 0; i < W.Cells; ++i)
			{
				if ((i < DoorCell.X || i > DoorCell.Y) && IsWindowCell(W.Cells, i))
				{
					const double Mid = i * Cell + Cell * 0.5;
					W.Openings.Emplace(Mid - WindowHalfWidth, Mid + WindowHalfWidth);
				}
			}
			if (W.DoorAlong >= 0.0)
			{
				W.Openings.Emplace(W.DoorAlong - DoorHalfWidth - DoorClearance, W.DoorAlong + DoorHalfWidth + DoorClearance);
			}
		}

		BuildBase();
		BuildWalls();
		BuildRoof();
		BuildGables();
		TArray<double> TrussU;
		BuildCeiling(TrussU);
		BuildDecor(TrussU);
	}

	FVector FGuildShellBuilder::ToWorld(double U, double V, double Z) const
	{
		return bSwap ? FVector(Span - V, U, Z) : FVector(U, V, Z);
	}

	void FGuildShellBuilder::Add(FGuildShellMeshPoints& Out, const FSoftObjectPath& Mesh, double U, double V, double Z, double Yaw,
		FVector Scale, double Pitch)
	{
		Out.Transforms.Emplace(FRotator(Pitch, Yaw + (bSwap ? 90.0 : 0.0), 0.0), ToWorld(U, V, Z), Scale);
		Out.Meshes.Add(Mesh);
	}

	void FGuildShellBuilder::Beam(const FSoftObjectPath& Mesh, double U, double V, double Z, double Yaw, double Pitch, double Length)
	{
		Add(Blocking, Mesh, U, V, Z, Yaw, FVector(Length / Cell, 1.0, 1.0), Pitch);
	}

	FVector2D FGuildShellBuilder::OnWall(int32 Side, double Along, double Lateral) const
	{
		const FWall& W = Walls[Side];
		return W.Start + W.Dir * Along + W.Inward() * Lateral;
	}

	double FGuildShellBuilder::AlongWall(int32 Side, double U, double V) const
	{
		const FWall& W = Walls[Side];
		return FVector2D::DotProduct(FVector2D(U, V) - W.Start, W.Dir);
	}

	void FGuildShellBuilder::AddOnWall(FGuildShellMeshPoints& Out, const FSoftObjectPath& Mesh, int32 Side, double Along, double Lateral,
		double Z, double YawOffset, FVector Scale, double Pitch)
	{
		const FVector2D P = OnWall(Side, Along, Lateral);
		Add(Out, Mesh, P.X, P.Y, Z, Walls[Side].Yaw + YawOffset, Scale, Pitch);
	}

	void FGuildShellBuilder::AddLight(TArray<FTransform>& Out, double U, double V, double Z) const
	{
		Out.Emplace(FRotator::ZeroRotator, ToWorld(U, V, Z));
	}

	bool FGuildShellBuilder::IsOpen(int32 Side, double Along, double HalfWidth) const
	{
		for (const FVector2D& R : Walls[Side].Openings)
		{
			if (Along + HalfWidth > R.X && Along - HalfWidth < R.Y)
			{
				return true;
			}
		}
		return false;
	}

	void FGuildShellBuilder::BuildBase()
	{
		// 바닥 타일: 피벗이 모서리이고 +X·-Y로 뻗는다. 무늬 방향이 돌지 않게 월드 기준으로 놓는다.
		for (int32 X = 0; X < Width; ++X)
		{
			for (int32 Y = 0; Y < Height; ++Y)
			{
				NoCollision.Transforms.Emplace(FVector(X * Cell,(Y + 1) * Cell, 0.0));
				NoCollision.Meshes.Add(Mesh::Floor);
			}
		}

		for (int32 Side = 0; Side < 4; ++Side)
		{
			const FWall& W = Walls[Side];
			// 모서리 기둥: 이 벽과 앞 벽의 두께 가운데선이 만나는 점.
			const FVector2D Corner = W.Start - (W.Inward() + Walls[(Side + 3) % 4].Inward()) * CornerInset;
			Add(NoCollision, Mesh::Column, Corner.X, Corner.Y, 0.0, W.Yaw, ColumnScale);
			// 벽 윗단 안쪽 띠보.
			for (int32 i = 0; i < W.Cells; ++i)
			{
				AddOnWall(NoCollision, Mesh::Rafter, Side, i * Cell, WallFace + BeamHalf, WallPlateZ);
			}
		}
	}

	void FGuildShellBuilder::BuildWalls()
	{
		for (int32 Side = 0; Side < 4; ++Side)
		{
			const FWall& W = Walls[Side];
			const double C = W.DoorAlong;
			const FIntPoint DoorCell = DoorCells(C);

			for (int32 i = 0; i < W.Cells; ++i)
			{
				if (i >= DoorCell.X && i <= DoorCell.Y)
				{
					continue;
				}
				AddOnWall(Blocking, IsWindowCell(W.Cells, i) ? Mesh::WindowWall : Mesh::Wall, Side, i * Cell, 0.0, 0.0);
			}
			if (C < 0.0)
			{
				continue;
			}

			// 문 양옆 조각: 비운 칸의 남은 폭을 채우고 문 쪽으로 5 겹친다.
			const bool bBoundary = DoorCell.Y > DoorCell.X; // 문이 칸 경계에 걸려 두 칸을 비움
			const double FillerWidth = bBoundary ? Cell : Cell * 0.5;
			const double Gap = FillerWidth - DoorHalfWidth + SeamOverlap;
			const FSoftObjectPath& Filler = bBoundary ? Mesh::Wall : Mesh::WallHalf;
			const FVector FillerScale(Gap / FillerWidth, 1.0, 1.0);
			AddOnWall(Blocking, Filler, Side, C - DoorHalfWidth + SeamOverlap - Gap, 0.0, 0.0, 0.0, FillerScale);
			AddOnWall(Blocking, Filler, Side, C + DoorHalfWidth - SeamOverlap, 0.0, 0.0, 0.0, FillerScale);

			// 문 위 벽(충돌 있음), 문틀·문짝(통로를 막지 않게 충돌 없음). Hearthvale 원본 배치 오프셋을 문 중심 기준으로 옮김.
			AddOnWall(Blocking, Mesh::DoorCap, Side, C - DoorHalfWidth, 0.0, 0.0);
			AddOnWall(NoCollision, Mesh::DoorFrame, Side, C, -10.0, 0.0);
			AddOnWall(NoCollision, Mesh::DoorLeft, Side, C - 86.0, -10.0, 91.4, 120.0);
			AddOnWall(NoCollision, Mesh::DoorRight, Side, C + 78.0, -10.0, 91.4, -120.0);
		}
	}
}
