#include "Building/GuildShellBuilder.h"

using namespace GuildShell;

namespace
{
	constexpr double EaveRun = 100.0;  // 처마판 수평 길이
	constexpr double VergeLen = 75.0;  // 박공 쪽 지붕 돌출
	const FVector MirrorX(-1.0, 1.0, 1.0);
}

void FGuildShellBuilder::BuildRoof()
{
	// 경사판·처마·쐐기·용마루: A면은 V=0 벽에서, B면은 V=Span 벽에서 올라간다(B는 180도 돌려 같은 부품 사용).
	for (int32 i = 0; i < L; ++i)
	{
		const double U0 = i * Cell;
		const double U1 = U0 + Cell;
		for (int32 k = 0; k < S; ++k)
		{
			Add(Blocking, Mesh::Slope, U0, k * Run, RoofBase + k * Run, 0.0);
			Add(Blocking, Mesh::Slope, U1, Span - k * Run, RoofBase + k * Run, 180.0);
		}
		Add(Blocking, Mesh::Eave, U0, -EaveRun, RoofBase - EaveRun, 0.0);
		Add(Blocking, Mesh::Eave, U1, Span + EaveRun, RoofBase - EaveRun, 180.0);
		Add(Blocking, Mesh::EaveFill, U0, 0.0, WallTop, 0.0);
		Add(Blocking, Mesh::EaveFill, U1, Span, WallTop, 180.0);
		Add(Blocking, Mesh::Ridge, U0, Half, RoofBase + Half, 0.0);
	}

	// 박공 쪽 돌출: 박공 끝판이 바깥 끝에 오도록 끝마다 X 부호를 바꾼다.
	for (int32 k = 0; k < S; ++k)
	{
		const double Z = RoofBase + k * Run;
		Add(Blocking, Mesh::SlopeVerge, -VergeLen, k * Run, Z, 0.0);
		Add(Blocking, Mesh::SlopeVerge, Len + VergeLen, k * Run, Z, 0.0, MirrorX);
		Add(Blocking, Mesh::SlopeVerge, -VergeLen, Span - k * Run, Z, 180.0, MirrorX);
		Add(Blocking, Mesh::SlopeVerge, Len + VergeLen, Span - k * Run, Z, 180.0);
	}
	Add(Blocking, Mesh::EaveVerge, -VergeLen, -EaveRun, RoofBase - EaveRun, 0.0);
	Add(Blocking, Mesh::EaveVerge, Len + VergeLen, -EaveRun, RoofBase - EaveRun, 0.0, MirrorX);
	Add(Blocking, Mesh::EaveVerge, -VergeLen, Span + EaveRun, RoofBase - EaveRun, 180.0, MirrorX);
	Add(Blocking, Mesh::EaveVerge, Len + VergeLen, Span + EaveRun, RoofBase - EaveRun, 180.0);
	Add(Blocking, Mesh::Ridge, -VergeLen, Half, RoofBase + Half, 0.0, FVector(VergeLen / Cell, 1.0, 1.0));
	Add(Blocking, Mesh::Ridge, Len, Half, RoofBase + Half, 0.0, FVector(VergeLen / Cell, 1.0, 1.0));
}

void FGuildShellBuilder::BuildGables()
{
	// 박공 창: 가운데 300 폭, 2층(300) 단위. 아랫단은 짧은 변 3칸 이상이면 큰 창,
	// 위 단은 창 위에 깃발 자리(300)를 남길 수 있을 때만 좁은 창.
	int32 WindowPairs = 0;
	while (S >= (WindowPairs == 0 ? 3 : 2 * WindowPairs + 5))
	{
		++WindowPairs;
	}
	const double WindowTop = RoofBase + WindowPairs * Cell;
	auto IsWindowBlock = [&](int32 Row, double BandStart)
	{
		return Row < 2 * WindowPairs && BandStart >= Half - Run - 1.0 && BandStart <= Half + 1.0;
	};
	const FVector BandScale(1.0, 1.0, GableBase / Run);

	// 박공벽 두 개: U=0 벽은 yaw -90(블록 X가 -V로, 바깥이 -U), U=Len 벽은 yaw 90(바깥이 +U).
	for (int32 End = 0; End < 2; ++End)
	{
		const double U = End == 0 ? 0.0 : Len;
		const double Yaw = End == 0 ? -90.0 : 90.0;
		const double Out = End == 0 ? -1.0 : 1.0;
		// 블록(150, 두께 40)이 V∈[A, A+150]을 덮게 하는 피벗 V.
		auto BlockV = [&](double A) { return End == 0 ? A + Run : A; };

		// 층 r은 양 끝이 150(r+1)씩 줄고, 끝은 삼각형(빗변이 지붕 밑면을 따라감). 창 자리는 비운다.
		for (int32 r = 0; r < S; ++r)
		{
			const double Z = RoofBase + r * Run;
			for (int32 j = 0; j < 2 * (S - r - 1); ++j)
			{
				const double A = Run * (r + 1) + Run * j;
				if (!IsWindowBlock(r, A))
				{
					Add(Blocking, Mesh::GableSquare, U, BlockV(A), Z, Yaw);
				}
			}
			Add(Blocking, Mesh::GableTri, U, r * Run, Z, Yaw, End == 0 ? MirrorX : FVector::OneVector);
			Add(Blocking, Mesh::GableTri, U, Span - r * Run, Z, Yaw, End == 0 ? FVector::OneVector : MirrorX);
		}
		// 벽 윗면(400)~지붕 밑면 시작(444) 띠는 판자.
		for (int32 j = 0; j < 2 * S; ++j)
		{
			Add(Blocking, Mesh::GableBand, U, BlockV(j * Run), WallTop, Yaw, BandScale);
		}

		for (int32 p = 0; p < WindowPairs; ++p)
		{
			// Tavern_D 창 벽은 +Y면이 회벽·아치 목재, -Y면이 돌이다. 회벽이 바깥을 보게 벽 방향과 반대로 돌리고,
			// 두께(-43~6)가 박공면에 맞게 37만큼 바깥으로 옮긴다.
			Add(Blocking, p == 0 ? Mesh::GableWindow : Mesh::GableWindowThin, U + Out * 37.0, End == 0 ? Half - Run : Half + Run,
				RoofBase + p * Cell, -Yaw);
		}

		// 목골조: 앞뒤 면(블록 두께 40 바깥 + 보 반두께 3, 안쪽 면 + 3)에 붙인다.
		// 가로대·기둥·버팀대가 겹치는 면이 한 평면에 오지 않게 0.5씩 띄운다.
		for (const double Face : { Out, -Out })
		{
			const double Base = Face == Out ? 43.0 : 3.0;
			auto FaceU = [&](double Lift) { return U + Face * (Base + Lift); };

			// 층마다 가로대(150 간격 이음매를 가린다). 창 가운데를 지나는 줄은 창 칸만 비운다.
			for (int32 r = 0; r < S; ++r)
			{
				const bool bMidWindow = (r % 2 == 1) && r < 2 * WindowPairs;
				for (int32 m = 0; m < S - r; ++m)
				{
					const double V0 = r * Run + m * Cell;
					if (bMidWindow && V0 < Half + Run && V0 + Cell > Half - Run)
					{
						continue;
					}
					Beam(Mesh::Timber, FaceU(0.0), V0, RoofBase + r * Run, 90.0, 0.0, Cell);
				}
			}
			// 창 양옆 기둥, 창 위 가운데 기둥.
			if (WindowPairs > 0)
			{
				Beam(Mesh::Timber, FaceU(0.5), Half - Run, RoofBase, 90.0, 90.0, WindowTop - RoofBase);
				Beam(Mesh::Timber, FaceU(0.5), Half + Run, RoofBase, 90.0, 90.0, WindowTop - RoofBase);
			}
			const double KingPostTop = RoofBase + Half - BeamHalf;
			if (KingPostTop - WindowTop > 50.0)
			{
				Beam(Mesh::Timber, FaceU(0.5), Half, WindowTop, 90.0, 90.0, KingPostTop - WindowTop);
			}
			// 45도 버팀대: 바깥 아래에서 창(가운데) 위 모서리로.
			if (S >= 3)
			{
				Beam(Mesh::Timber, FaceU(1.0), Half - 3.0 * Run, RoofBase, 90.0, 45.0, Cell * UE_DOUBLE_SQRT_2);
				Beam(Mesh::Timber, FaceU(1.0), Half + 3.0 * Run, RoofBase, -90.0, 45.0, Cell * UE_DOUBLE_SQRT_2);
			}
		}

		// 깃발: 창 위 두 번째 가로대에 건다(아래로 약 2m). 바깥 면만.
		const int32 BannerRow = 2 * WindowPairs + 2;
		if (S >= BannerRow + 1)
		{
			Add(Blocking, Mesh::GableBanner, U + Out * 48.0, Half, RoofBase + BannerRow * Run - 12.0, 90.0);
		}
	}
}
