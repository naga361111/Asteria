#include "Building/GuildShellBuilder.h"

namespace GuildShell
{
	namespace
	{
		constexpr int32 TrussEvery = 2;         // 트러스 간격(칸). 양 끝 박공벽 자리에는 두지 않는다.
		constexpr double RafterInset = BeamHalf * UE_DOUBLE_INV_SQRT_2; // 서까래 중심을 판재 밑면에서 수직 10만큼 안쪽으로
		constexpr double ChainLength = 68.0;    // SM_Chain_Line_a 한 마디
		constexpr int32 ChainLinks = 2;
		constexpr double LanternHeight = 44.0;  // SM_Lantern_c: 바닥이 피벗, 위 끝 고리까지 44
		constexpr double LanternLightDrop = 10.0; // 매단 랜턴 조명: 랜턴 바닥 아래 거리
		constexpr double SconceZ = 280.0;
		constexpr double WallBannerTop = 392.0; // 벽 윗단 나무 띠 위쪽에 건다
	}

	void FGuildShellBuilder::BuildCeiling(TArray<double>& OutTrussU)
	{
		// 평천장: 경사 천장이 CeilingRise까지만 오르고 가운데(Flat0~Flat1)는 평평. 짧은 변이 3칸보다 작으면 경사 천장만.
		const double CeilingZ = RoofBase + CeilingRise;
		const double Flat0 = CeilingRise;
		const double Flat1 = Span - CeilingRise;
		if (bFlat)
		{
			const FVector Flip(1.0, 1.0, -1.0);
			for (int32 i = 0; i < L; ++i)
			{
				for (double V = Flat0; V < Flat1 - 1.0; V += Cell)
				{
					// 판재는 모서리 피벗에서 +X·-Y로 뻗는다 → V+300 모서리에 놓으면 [V, V+300]을 덮는다. Z를 뒤집어 아래를 보게.
					Add(Blocking, Mesh::CeilingPlank, i * Cell, V + Cell, CeilingZ, 0.0, Flip);
					// 판재는 한 면(두께 0.5)이라 뒤집으면 윗면이 뒷면이 되어 햇빛 그림자를 안 만든다. 위를 보는 판으로 덮는다.
					Add(Blocking, Mesh::CeilingPlank, i * Cell, V + Cell, CeilingZ + 1.0, 0.0);
				}
				// 테두리 보(경사 천장과 만나는 선)와 가운데 등뼈 보.
				for (const double V : { Flat0, Half, Flat1 })
				{
					Add(Blocking, Mesh::TieBeam, i * Cell, V, CeilingZ - BeamHalf, 0.0);
				}
			}
		}

		// 트러스: 가로보 + (평천장이면 두 기둥과 이음보, 아니면 가운데 기둥) + 경사 천장 밑 서까래.
		for (double U = TrussEvery * Cell; U < Len - 1.0; U += TrussEvery * Cell)
		{
			OutTrussU.Add(U);
		}
		const double PostBottom = TieBeamZ + BeamHalf;
		for (const double U : OutTrussU)
		{
			for (int32 c = 0; c < S; ++c)
			{
				Add(Blocking, Mesh::TieBeam, U, c * Cell, TieBeamZ, 90.0);
			}
			if (bFlat)
			{
				for (double V = Flat0; V < Flat1 - 1.0; V += Cell)
				{
					Add(Blocking, Mesh::TieBeam, U, V, CeilingZ - BeamHalf, 90.0);
				}
				Beam(Mesh::Rafter, U, Flat0, PostBottom, 0.0, 90.0, CeilingZ - 2.0 * BeamHalf - PostBottom);
				Beam(Mesh::Rafter, U, Flat1, PostBottom, 0.0, 90.0, CeilingZ - 2.0 * BeamHalf - PostBottom);
			}
			else
			{
				Beam(Mesh::Rafter, U, Half, PostBottom, 0.0, 90.0, RoofBase + Half - 2.0 * BeamHalf - PostBottom);
			}
			for (int32 k = 0; k < RoofRows; ++k)
			{
				const double Z = RoofBase + k * Run - RafterInset;
				Beam(Mesh::Rafter, U, k * Run + RafterInset, Z, 90.0, 45.0, Run * UE_DOUBLE_SQRT_2);
				Beam(Mesh::Rafter, U, Span - k * Run - RafterInset, Z, -90.0, 45.0, Run * UE_DOUBLE_SQRT_2);
			}
		}

		// 평천장 장선: 트러스 사이 칸 경계마다 테두리 보 사이를 가로지른다.
		if (bFlat)
		{
			for (int32 i = 1; i < L; ++i)
			{
				const double U = i * Cell;
				if (!OutTrussU.ContainsByPredicate([U](double T) { return FMath::IsNearlyEqual(T, U, 1.0); }))
				{
					Beam(Mesh::Rafter, U, Flat0, CeilingZ - BeamHalf, 90.0, 0.0, Flat1 - Flat0);
				}
			}
		}
	}

	void FGuildShellBuilder::BuildDecor(const TArray<double>& TrussU)
	{
		const double PostV = WallFace + BeamHalf;
		for (const double U : TrussU)
		{
			// 가로보 밑 랜턴 두 개(사슬 두 마디).
			for (const double V : { Half - Span * 0.25, Half + Span * 0.25 })
			{
				double Z = TieBeamZ - BeamHalf;
				for (int32 n = 0; n < ChainLinks; ++n, Z -= ChainLength)
				{
					Add(Blocking, Mesh::Chain, U, V, Z, 0.0);
				}
				Add(Blocking, Mesh::HangingLantern, U, V, Z - LanternHeight, 0.0);
				// 랜턴 속에 두면 랜턴 틀이 그림자로 빛을 가린다 → 바닥 바로 아래에 둔다(유리는 자체 발광).
				AddLight(Lights, U, V, Z - LanternHeight - LanternLightDrop);
			}
			// 긴 벽(A면 V=0 / B면 V=Span) 기둥 + 45도 버팀대, 기둥에 벽 등. 문·창 자리는 피한다.
			for (const int32 Side : { 0, 2 })
			{
				const double Along = AlongWall(Side, U, 0.0);
				if (IsOpen(Side, Along, 40.0))
				{
					continue;
				}
				const FVector2D Post = OnWall(Side, Along, PostV);
				Beam(Mesh::Rafter, Post.X, Post.Y, 0.0, 0.0, 90.0, TieBeamZ - BeamHalf);
				AddOnWall(Blocking, Mesh::Rafter, Side, Along, PostV + BeamHalf, TieBeamZ - BeamHalf - 90.0, 90.0,
					FVector(127.0 / Cell, 1.0, 1.0), 45.0);
				AddOnWall(Blocking, Mesh::Sconce, Side, Along, PostV + BeamHalf, SconceZ, 90.0);
				const FVector2D Light = OnWall(Side, Along, PostV + 45.0);
				AddLight(SconceLights, Light.X, Light.Y, SconceZ);
			}
		}

		// 기둥 사이(칸 가운데) 긴 벽에 깃발.
		int32 BannerIndex = 0;
		double Prev = 0.0;
		TArray<double> Bays = TrussU;
		Bays.Add(Len);
		for (const double U : Bays)
		{
			const double Mid = (Prev + U) * 0.5;
			Prev = U;
			for (const int32 Side : { 0, 2 })
			{
				const double Along = AlongWall(Side, Mid, 0.0);
				if (!IsOpen(Side, Along, 60.0))
				{
					const FSoftObjectPath& Banner = Mesh::WallBanners[BannerIndex++ % UE_ARRAY_COUNT(Mesh::WallBanners)];
					AddOnWall(Blocking, Banner, Side, Along, WallFace + 2.0, WallBannerTop);
				}
			}
		}

		// 박공 벽(짧은 변) 안쪽: 1/4 지점에서 가장 가까운 칸 경계(창 가운데를 가리지 않게)에 벽 등.
		const double SconceV = FMath::RoundToDouble(Span * 0.25 / Cell) * Cell;
		if (SconceV <= 0.0 || SconceV >= Span * 0.5)
		{
			return;
		}
		for (const int32 Side : { 3, 1 })
		{
			// 두 자리가 벽 가운데 기준 대칭이라 벽 진행 방향과 상관없이 같은 Along 값을 쓴다.
			for (const double Along : { SconceV, Span - SconceV })
			{
				if (!IsOpen(Side, Along, 40.0))
				{
					AddOnWall(Blocking, Mesh::Sconce, Side, Along, WallFace, SconceZ, 90.0);
					const FVector2D Light = OnWall(Side, Along, WallFace + 35.0);
					AddLight(SconceLights, Light.X, Light.Y, SconceZ);
				}
			}
		}
	}
}
