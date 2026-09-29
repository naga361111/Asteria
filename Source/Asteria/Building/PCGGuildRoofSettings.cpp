#include "PCGGuildRoofSettings.h"

#include "PCGContext.h"
#include "PCGPin.h"
#include "Data/PCGBasePointData.h"
#include "Engine/StaticMesh.h"
#include "Helpers/PCGHelpers.h"
#include "Metadata/PCGMetadata.h"

namespace
{
	// Hearthvale 300 격자·Tavern_C 벽 400 기준. 지붕 부품 치수는 Tools/Blender/guild_roof_parts.py와 같아야 한다.
	constexpr double Cell = 300.0;
	constexpr double Run = 150.0;          // 경사판 한 칸 수평(=수직) 길이, 45도
	constexpr double WallTop = 400.0;      // Tavern_C
	constexpr double GableBase = 44.0;     // 벽 윗면~지붕 밑면 시작 띠. 지붕 밑면은 벽 바깥 윗모서리(Y -44)에서 45도.
	constexpr double RoofBase = WallTop + GableBase;
	constexpr double EaveRun = 100.0;      // 처마판 수평 길이
	constexpr double VergeLen = 75.0;      // 박공 쪽 지붕 돌출
	constexpr double TieBeamZ = 430.0;     // 트러스 가로보 중심(두께 20 → 420~440)
	constexpr double BeamHalf = 10.0;      // 20×20 보·80폭 보의 반두께
	constexpr double RafterInset = 7.0710678; // 서까래(20×20) 중심을 판재 밑면에서 수직 10만큼 안쪽으로
	constexpr double WallFace = 6.0;       // Tavern_C 실내 면(외곽선에서 안쪽 6)
	constexpr double ChainLength = 68.0;   // SM_Chain_Line_a 한 마디
	constexpr int32 ChainLinks = 2;
	constexpr double LanternHeight = 44.0; // SM_Lantern_c: 바닥이 피벗, 위 끝 고리까지 44
	constexpr double SconceZ = 280.0;
	constexpr double WallBannerTop = 392.0; // 벽 윗단 나무 띠 위쪽에 건다
	constexpr double DoorClearance = 40.0; // 장식은 문 양옆으로 이만큼 더 비운다
	constexpr double DoorHalfWidth = 90.0; // 문 위 벽(Door_Frame_B_Cap) 폭 180의 절반
	constexpr double SeamOverlap = 5.0;    // 문 옆 조각이 문 쪽으로 겹치는 폭(끝 모서리가 문틀 뒤로 숨음)

	const FName MeshAttribute(TEXT("Mesh"));
	const FName LightsPin(TEXT("Lights"));
	const FName DoorPartsPin(TEXT("DoorParts"));

	struct FRoofPoint
	{
		FSoftObjectPath Mesh;
		FTransform Transform;
	};

	// 변 시작에서 잰 문 중심. 벽 중심 + 오프셋×300이고 모서리 칸에는 닿지 않게 멈춘다.
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

	UPCGBasePointData* MakePointData(FPCGContext* Context, const TArray<FTransform>& Transforms, const TArray<FSoftObjectPath>* Meshes)
	{
		UPCGBasePointData* PointData = FPCGContext::NewPointData_AnyThread(Context);
		PointData->SetNumPoints(Transforms.Num(), /*bInitializeValues=*/false);
		PointData->AllocateProperties(EPCGPointNativeProperties::All);
		FPCGMetadataAttribute<FSoftObjectPath>* MeshAttr = Meshes
			? PointData->Metadata->FindOrCreateAttribute<FSoftObjectPath>(MeshAttribute, FSoftObjectPath(), false, false)
			: nullptr;

		FPCGPointValueRanges Ranges(PointData, /*bAllocate=*/false);
		for (int32 Index = 0; Index < Transforms.Num(); ++Index)
		{
			Ranges.TransformRange[Index] = Transforms[Index];
			Ranges.DensityRange[Index] = 1.0f;
			Ranges.BoundsMinRange[Index] = FVector(-1.0);
			Ranges.BoundsMaxRange[Index] = FVector(1.0);
			Ranges.ColorRange[Index] = FVector4(1.0);
			Ranges.SteepnessRange[Index] = 1.0f;
			Ranges.SeedRange[Index] = PCGHelpers::ComputeSeedFromPosition(Transforms[Index].GetLocation());
			Ranges.MetadataEntryRange[Index] = PCGInvalidEntryKey;
			if (MeshAttr)
			{
				PointData->Metadata->InitializeOnSet(Ranges.MetadataEntryRange[Index]);
				MeshAttr->SetValue(Ranges.MetadataEntryRange[Index], (*Meshes)[Index]);
			}
		}
		return PointData;
	}
}

UPCGGuildRoofSettings::UPCGGuildRoofSettings()
{
	const FString Roof = TEXT("/Game/Map/Building/Meshes/Roof/SM_GuildRoof_");
	const FString Hv = TEXT("/Game/Hearthvale/Meshes/");
	WallMesh = FSoftObjectPath(Hv + TEXT("Walls/SM_Wall_Tavern_C.SM_Wall_Tavern_C"));
	WallHalfMesh = FSoftObjectPath(Hv + TEXT("Walls/SM_Wall_Tavern_C_Half.SM_Wall_Tavern_C_Half"));
	DoorCapMesh = FSoftObjectPath(Hv + TEXT("Walls/SM_Door_Frame_B_Cap.SM_Door_Frame_B_Cap"));
	DoorFrameMesh = FSoftObjectPath(Hv + TEXT("Doors/SM_Door_Frame_A.SM_Door_Frame_A"));
	DoorLeftMesh = FSoftObjectPath(Hv + TEXT("Doors/SM_Door_A_LT.SM_Door_A_LT"));
	DoorRightMesh = FSoftObjectPath(Hv + TEXT("Doors/SM_Door_A_RT.SM_Door_A_RT"));
	SlopeMesh = FSoftObjectPath(Roof + TEXT("Slope.SM_GuildRoof_Slope"));
	SlopeVergeMesh = FSoftObjectPath(Roof + TEXT("SlopeVerge.SM_GuildRoof_SlopeVerge"));
	EaveMesh = FSoftObjectPath(Roof + TEXT("Eave.SM_GuildRoof_Eave"));
	EaveVergeMesh = FSoftObjectPath(Roof + TEXT("EaveVerge.SM_GuildRoof_EaveVerge"));
	RidgeMesh = FSoftObjectPath(Roof + TEXT("Ridge.SM_GuildRoof_Ridge"));
	GableSquareMesh = FSoftObjectPath(Roof + TEXT("GableSquare.SM_GuildRoof_GableSquare"));
	GableTriMesh = FSoftObjectPath(Roof + TEXT("GableTri.SM_GuildRoof_GableTri"));
	EaveFillMesh = FSoftObjectPath(Roof + TEXT("EaveFill.SM_GuildRoof_EaveFill"));
	GableSquarePlasterMesh = FSoftObjectPath(Roof + TEXT("GableSquare_Plaster.SM_GuildRoof_GableSquare_Plaster"));
	GableTriPlasterMesh = FSoftObjectPath(Roof + TEXT("GableTri_Plaster.SM_GuildRoof_GableTri_Plaster"));
	WindowMesh = FSoftObjectPath(Hv + TEXT("Walls/SM_Wall_Tavern_D_Window.SM_Wall_Tavern_D_Window"));
	WindowThinMesh = FSoftObjectPath(Hv + TEXT("Walls/SM_Wall_Tavern_D_Window_Thin.SM_Wall_Tavern_D_Window_Thin"));
	TimberMesh = FSoftObjectPath(Hv + TEXT("Beams/SM_WoodBeam_A.SM_WoodBeam_A"));
	BannerMesh = FSoftObjectPath(Hv + TEXT("Fabric/SM_Banner_c.SM_Banner_c"));
	TieBeamMesh = FSoftObjectPath(Hv + TEXT("Beams/sm_beam_wide_300.sm_beam_wide_300"));
	RafterMesh = FSoftObjectPath(Hv + TEXT("Beams/SM_WoodBeam_B.SM_WoodBeam_B"));
	CeilingPlankMesh = FSoftObjectPath(Hv + TEXT("Floor/SM_PlanksFloor_A.SM_PlanksFloor_A"));
	HangingLanternMesh = FSoftObjectPath(Hv + TEXT("Details/SM_Lantern_c.SM_Lantern_c"));
	ChainMesh = FSoftObjectPath(Hv + TEXT("Chain/SM_Chain_Line_a.SM_Chain_Line_a"));
	SconceMesh = FSoftObjectPath(Hv + TEXT("Lamp/SM_Latern_C.SM_Latern_C"));
	for (const TCHAR* Name : { TEXT("a"), TEXT("d"), TEXT("g"), TEXT("e") })
	{
		WallBannerMeshes.Add(TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(FString::Printf(TEXT("%sFabric/SM_Banner_%s.SM_Banner_%s"), *Hv, Name, Name))));
	}
}

TArray<FPCGPinProperties> UPCGGuildRoofSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> Pins;
	Pins.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Point);
	Pins.Emplace(LightsPin, EPCGDataType::Point);
	Pins.Emplace(DoorPartsPin, EPCGDataType::Point);
	return Pins;
}

FPCGElementPtr UPCGGuildRoofSettings::CreateElement() const
{
	return MakeShared<FPCGGuildRoofElement>();
}

bool FPCGGuildRoofElement::ExecuteInternal(FPCGContext* Context) const
{
	const UPCGGuildRoofSettings* Settings = Context->GetInputSettings<UPCGGuildRoofSettings>();
	check(Settings);

	const int32 Width = FMath::Max(1, Settings->ShellWidth);
	const int32 Height = FMath::Max(1, Settings->ShellHeight);

	// 기준 좌표(U=용마루 방향, V=경사 방향)에서 만들고, 세로가 더 길면 90도 돌려 월드에 놓는다.
	const bool bSwap = Width < Height;
	const int32 L = FMath::Max(Width, Height);
	const int32 S = FMath::Min(Width, Height);
	const double Len = L * Cell;
	const double Span = S * Cell;
	const double Half = S * Run;

	TArray<FTransform> Transforms;
	TArray<FSoftObjectPath> Meshes;
	TArray<FTransform> Lights;
	TArray<FTransform> DoorTransforms;
	TArray<FSoftObjectPath> DoorMeshes;
	auto ToWorld = [&](double U, double V, double Z)
	{
		return bSwap ? FVector(Span - V, U, Z) : FVector(U, V, Z);
	};
	auto Add = [&](const TSoftObjectPtr<UStaticMesh>& Mesh, double U, double V, double Z, double Yaw, FVector Scale = FVector::OneVector, double Pitch = 0.0)
	{
		Transforms.Emplace(FRotator(Pitch, Yaw + (bSwap ? 90.0 : 0.0), 0.0), ToWorld(U, V, Z), Scale);
		Meshes.Add(Mesh.ToSoftObjectPath());
	};
	auto AddLight = [&](double U, double V, double Z)
	{
		Lights.Emplace(FRotator::ZeroRotator, ToWorld(U, V, Z));
	};
	const FVector MirrorX(-1.0, 1.0, 1.0);

	// ---- 외곽 벽·문 (월드 좌표, 외곽선을 시계 방향으로: 실내가 진행 방향 +Y) ----
	// 0=NegY(y=0, +X로), 1=PosX(x=W, +Y로), 2=PosY(y=H, -X로), 3=NegX(x=0, -Y로).
	struct FSide { FVector Start; FVector Dir; int32 Cells; bool bDoor; int32 Offset; };
	const FSide Sides[4] = {
		{ FVector(0.0, 0.0, 0.0), FVector(1.0, 0.0, 0.0), Width, Settings->bDoorNegY, Settings->DoorNegYOffset },
		{ FVector(Width * Cell, 0.0, 0.0), FVector(0.0, 1.0, 0.0), Height, Settings->bDoorPosX, Settings->DoorPosXOffset },
		{ FVector(Width * Cell, Height * Cell, 0.0), FVector(-1.0, 0.0, 0.0), Width, Settings->bDoorPosY, Settings->DoorPosYOffset },
		{ FVector(0.0, Height * Cell, 0.0), FVector(0.0, -1.0, 0.0), Height, Settings->bDoorNegX, Settings->DoorNegXOffset },
	};
	double DoorAlong[4] = { -1.0, -1.0, -1.0, -1.0 };
	for (int32 Side = 0; Side < 4; ++Side)
	{
		if (Sides[Side].bDoor)
		{
			DoorAlong[Side] = DoorCenter(Sides[Side].Cells, Sides[Side].Offset);
		}
	}

	if (Settings->bWalls)
	{
		for (int32 Side = 0; Side < 4; ++Side)
		{
			const FSide& W = Sides[Side];
			const FVector Inward(-W.Dir.Y, W.Dir.X, 0.0);
			const double WallYaw = FMath::RadiansToDegrees(FMath::Atan2(W.Dir.Y, W.Dir.X));
			// Along = 변 시작에서 진행 방향 거리, Lateral = 로컬 +Y(실내 쪽) 거리.
			auto Place = [&](TArray<FTransform>& OutT, TArray<FSoftObjectPath>& OutM, const TSoftObjectPtr<UStaticMesh>& Mesh,
				double Along, double Lateral, double Z, double YawOffset = 0.0, FVector Scale = FVector::OneVector)
			{
				OutT.Emplace(FRotator(0.0, WallYaw + YawOffset, 0.0), W.Start + W.Dir * Along + Inward * Lateral + FVector(0.0, 0.0, Z), Scale);
				OutM.Add(Mesh.ToSoftObjectPath());
			};

			// 문이 차지하는 칸: 짝수 칸 벽이면 경계 양옆 두 칸, 홀수면 한 칸.
			const double C = DoorAlong[Side];
			const bool bDoor = C >= 0.0;
			const bool bBoundary = bDoor && FMath::IsNearlyZero(FMath::Fmod(C, Cell));
			const int32 First = !bDoor ? -1 : bBoundary ? FMath::RoundToInt(C / Cell) - 1 : FMath::FloorToInt(C / Cell);
			const int32 Last = !bDoor ? -1 : bBoundary ? First + 1 : First;

			for (int32 i = 0; i < W.Cells; ++i)
			{
				if (i < First || i > Last)
				{
					Place(Transforms, Meshes, Settings->WallMesh, i * Cell, 0.0, 0.0);
				}
			}
			if (!bDoor)
			{
				continue;
			}

			// 문 양옆 조각: 비운 칸의 남은 폭을 채우고 문 쪽으로 5 겹친다.
			const double Gap = (bBoundary ? Cell : Cell * 0.5) - DoorHalfWidth + SeamOverlap;
			const TSoftObjectPtr<UStaticMesh>& Filler = bBoundary ? Settings->WallMesh : Settings->WallHalfMesh;
			const double FillerWidth = bBoundary ? Cell : Cell * 0.5;
			const FVector FillerScale(Gap / FillerWidth, 1.0, 1.0);
			Place(Transforms, Meshes, Filler, C - DoorHalfWidth + SeamOverlap - Gap, 0.0, 0.0, 0.0, FillerScale);
			Place(Transforms, Meshes, Filler, C + DoorHalfWidth - SeamOverlap, 0.0, 0.0, 0.0, FillerScale);

			// 문 위 벽·문틀·문짝(원래 칸 시작 기준 오프셋 0 / 90 / 4·168을 문 중심 기준으로 옮김).
			Place(Transforms, Meshes, Settings->DoorCapMesh, C - DoorHalfWidth, 0.0, 0.0);
			Place(DoorTransforms, DoorMeshes, Settings->DoorFrameMesh, C, -10.0, 0.0);
			Place(DoorTransforms, DoorMeshes, Settings->DoorLeftMesh, C - 86.0, -10.0, 91.4, 120.0);
			Place(DoorTransforms, DoorMeshes, Settings->DoorRightMesh, C + 78.0, -10.0, 91.4, -120.0);
		}
	}

	// 문 자리(기준 좌표). 벽 기둥·벽 등·깃발이 피한다. 0=A면(V=0), 1=B면(V=Span), 2=U=0 박공, 3=U=Len 박공.
	TArray<FVector2D> DoorSpans[4];
	for (int32 Side = 0; Side < 4; ++Side)
	{
		if (DoorAlong[Side] < 0.0)
		{
			continue;
		}
		// 변 진행 거리 → 월드 X(0·2번 변) 또는 Y(1·3번 변). 2·3번 변은 반대 방향으로 진행한다.
		const double World = Side < 2 ? DoorAlong[Side] : Sides[Side].Cells * Cell - DoorAlong[Side];
		const FVector2D Along(World - DoorHalfWidth - DoorClearance, World + DoorHalfWidth + DoorClearance);
		// 회전하지 않으면 NegY/PosY가 A/B면, NegX/PosX가 U=0/U=Len 박공.
		// 회전하면 월드 X = Span - V, 월드 Y = U 이므로 PosX/NegX가 A/B면, NegY/PosY가 U=0/U=Len 박공.
		int32 Wall;
		FVector2D Range = Along;
		if (!bSwap)
		{
			Wall = Side == 0 ? 0 : Side == 2 ? 1 : Side == 3 ? 2 : 3;
		}
		else
		{
			Wall = Side == 1 ? 0 : Side == 3 ? 1 : Side == 0 ? 2 : 3;
			if (Wall >= 2)
			{
				Range = FVector2D(Span - Along.Y, Span - Along.X); // 월드 X → V
			}
		}
		DoorSpans[Wall].Add(Range);
	}

	// 경사판·처마·쐐기·용마루: A면은 V=0 벽에서, B면은 V=Span 벽에서 올라간다(B는 180도 돌려 같은 부품 사용).
	for (int32 i = 0; i < L; ++i)
	{
		const double U0 = i * Cell;
		const double U1 = U0 + Cell;
		for (int32 k = 0; k < S; ++k)
		{
			Add(Settings->SlopeMesh, U0, k * Run, RoofBase + k * Run, 0.0);
			Add(Settings->SlopeMesh, U1, Span - k * Run, RoofBase + k * Run, 180.0);
		}
		Add(Settings->EaveMesh, U0, -EaveRun, RoofBase - EaveRun, 0.0);
		Add(Settings->EaveMesh, U1, Span + EaveRun, RoofBase - EaveRun, 180.0);
		Add(Settings->EaveFillMesh, U0, 0.0, WallTop, 0.0);
		Add(Settings->EaveFillMesh, U1, Span, WallTop, 180.0);
		Add(Settings->RidgeMesh, U0, Half, RoofBase + Half, 0.0);
	}

	// 박공 쪽 돌출: 박공 끝판이 바깥 끝에 오도록 끝마다 X 부호를 바꾼다.
	for (int32 k = 0; k < S; ++k)
	{
		const double Z = RoofBase + k * Run;
		Add(Settings->SlopeVergeMesh, -VergeLen, k * Run, Z, 0.0);
		Add(Settings->SlopeVergeMesh, Len + VergeLen, k * Run, Z, 0.0, MirrorX);
		Add(Settings->SlopeVergeMesh, -VergeLen, Span - k * Run, Z, 180.0, MirrorX);
		Add(Settings->SlopeVergeMesh, Len + VergeLen, Span - k * Run, Z, 180.0);
	}
	Add(Settings->EaveVergeMesh, -VergeLen, -EaveRun, RoofBase - EaveRun, 0.0);
	Add(Settings->EaveVergeMesh, Len + VergeLen, -EaveRun, RoofBase - EaveRun, 0.0, MirrorX);
	Add(Settings->EaveVergeMesh, -VergeLen, Span + EaveRun, RoofBase - EaveRun, 180.0, MirrorX);
	Add(Settings->EaveVergeMesh, Len + VergeLen, Span + EaveRun, RoofBase - EaveRun, 180.0);
	Add(Settings->RidgeMesh, -VergeLen, Half, RoofBase + Half, 0.0, FVector(VergeLen / Cell, 1.0, 1.0));
	Add(Settings->RidgeMesh, Len, Half, RoofBase + Half, 0.0, FVector(VergeLen / Cell, 1.0, 1.0));

	// 박공 창: 가운데 300 폭, 2층(300) 단위. 아랫단(p=0)은 짧은 변 3칸 이상이면 큰 창,
	// 위 단은 창 위에 깃발 자리(300)를 남길 수 있을 때만 좁은 창.
	int32 WindowPairs = 0;
	if (Settings->bGableWindows)
	{
		while (S >= (WindowPairs == 0 ? 3 : 2 * WindowPairs + 5))
		{
			++WindowPairs;
		}
	}
	const double WindowTop = RoofBase + WindowPairs * Cell;
	auto IsWindowBlock = [&](int32 Row, double BandStart)
	{
		return Row < 2 * WindowPairs && BandStart >= Half - Run - 1.0 && BandStart <= Half + 1.0;
	};

	const TSoftObjectPtr<UStaticMesh>& GableSquare = Settings->bPlasterGable ? Settings->GableSquarePlasterMesh : Settings->GableSquareMesh;
	const TSoftObjectPtr<UStaticMesh>& GableTri = Settings->bPlasterGable ? Settings->GableTriPlasterMesh : Settings->GableTriMesh;
	const FVector BaseScale(1.0, 1.0, GableBase / Run);

	// 박공벽 두 개: U=0 벽은 yaw -90(블록 X가 -V로, 바깥이 -U), U=Len 벽은 yaw 90(바깥이 +U).
	for (int32 End = 0; End < 2; ++End)
	{
		const double U = End == 0 ? 0.0 : Len;
		const double Yaw = End == 0 ? -90.0 : 90.0;
		const double Out = End == 0 ? -1.0 : 1.0;
		// 블록이 V∈[A, A+150]을 덮게 하는 피벗 V.
		auto BlockV = [&](double A) { return End == 0 ? A + Run : A; };

		// 층 r은 양 끝이 150(r+1)씩 줄고, 끝은 삼각형(빗변이 지붕 밑면을 따라감).
		for (int32 r = 0; r < S; ++r)
		{
			const double Z = RoofBase + r * Run;
			for (int32 j = 0; j < 2 * (S - r - 1); ++j)
			{
				const double A = Run * (r + 1) + Run * j;
				if (!IsWindowBlock(r, A))
				{
					Add(GableSquare, U, BlockV(A), Z, Yaw);
				}
			}
			Add(GableTri, U, r * Run, Z, Yaw, End == 0 ? MirrorX : FVector::OneVector);
			Add(GableTri, U, Span - r * Run, Z, Yaw, End == 0 ? FVector::OneVector : MirrorX);
		}
		// 벽 윗면(400)~지붕 밑면 시작(444) 띠는 판자.
		for (int32 j = 0; j < 2 * S; ++j)
		{
			Add(Settings->GableSquareMesh, U, BlockV(j * Run), WallTop, Yaw, BaseScale);
		}

		for (int32 p = 0; p < WindowPairs; ++p)
		{
			// Tavern_D 창 벽은 +Y면이 회벽·아치 목재, -Y면이 돌이다. 회벽이 바깥을 보게 벽 방향과 반대로 돌리고,
			// 두께(-43~6)가 박공면에 맞게 37만큼 바깥으로 옮긴다.
			Add(p == 0 ? Settings->WindowMesh : Settings->WindowThinMesh, U + Out * 37.0, End == 0 ? Half - Run : Half + Run,
				RoofBase + p * Cell, -Yaw);
		}

		if (Settings->bGableTimber)
		{
			// 앞뒤 면에 붙인다. 가로대·기둥·버팀대가 겹치는 면이 한 평면에 오지 않게 0.5씩 띄운다.
			for (const double Face : { Out, -Out })
			{
				const double Base = Face == Out ? 43.0 : 3.0;
				auto FaceU = [&](double Lift) { return U + Face * (Base + Lift); };
				auto Horizontal = [&](double V0, double V1, double Z)
				{
					Add(Settings->TimberMesh, FaceU(0.0), V0, Z, 90.0, FVector((V1 - V0) / Cell, 1.0, 1.0));
				};
				auto Vertical = [&](double V, double Z0, double Z1)
				{
					Add(Settings->TimberMesh, FaceU(0.5), V, Z0, 90.0, FVector((Z1 - Z0) / Cell, 1.0, 1.0), 90.0);
				};

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
						Horizontal(V0, V0 + Cell, RoofBase + r * Run);
					}
				}
				// 창 양옆 기둥, 창 위 가운데 기둥.
				if (WindowPairs > 0)
				{
					Vertical(Half - Run, RoofBase, WindowTop);
					Vertical(Half + Run, RoofBase, WindowTop);
				}
				if (RoofBase + Half - 10.0 - WindowTop > 50.0)
				{
					Vertical(Half, WindowTop, RoofBase + Half - 10.0);
				}
				// 45도 버팀대: 바깥 아래에서 창(가운데) 위 모서리로.
				if (S >= 3)
				{
					const FVector BraceScale(Cell / UE_DOUBLE_INV_SQRT_2 / Cell, 1.0, 1.0);
					Add(Settings->TimberMesh, FaceU(1.0), Half - 3.0 * Run, RoofBase, 90.0, BraceScale, 45.0);
					Add(Settings->TimberMesh, FaceU(1.0), Half + 3.0 * Run, RoofBase, -90.0, BraceScale, 45.0);
				}
			}
		}

		// 깃발: 창 위 두 번째 가로대에 건다(아래로 약 2m). 바깥 면만.
		const int32 BannerRow = 2 * WindowPairs + 2;
		if (Settings->bGableBanner && S >= BannerRow + 1)
		{
			Add(Settings->BannerMesh, U + Out * 48.0, Half, RoofBase + BannerRow * Run - 12.0, 90.0);
		}
	}

	// ---- 실내 ----

	// 평천장: 경사 천장이 CeilingRise까지만 오르고 가운데(Flat0~Flat1)는 평평. 경사판 줄(150) 단위로 맞춘다.
	const double Rise = FMath::Max(Run, FMath::RoundToDouble(Settings->CeilingRise / Run) * Run);
	const bool bFlat = Settings->bFlatCeiling && Span - 2.0 * Rise >= Cell;
	const double CeilingZ = RoofBase + Rise;
	const double Flat0 = Rise;
	const double Flat1 = Span - Rise;
	if (bFlat)
	{
		const FVector Flip(1.0, 1.0, -1.0);
		for (int32 i = 0; i < L; ++i)
		{
			for (double V = Flat0; V < Flat1 - 1.0; V += Cell)
			{
				// 판재는 모서리 피벗에서 +X·-Y로 뻗는다 → V+300 모서리에 놓으면 [V, V+300]을 덮는다.
				Add(Settings->CeilingPlankMesh, i * Cell, FMath::Min(V + Cell, Flat1), CeilingZ, 0.0, Flip);
			}
		}
		// 테두리 보(경사 천장과 만나는 선)와 가운데 등뼈 보: 용마루 방향으로 칸마다.
		for (int32 i = 0; i < L; ++i)
		{
			for (const double V : { Flat0, Half, Flat1 })
			{
				Add(Settings->TieBeamMesh, i * Cell, V, CeilingZ - BeamHalf, 0.0);
			}
		}
	}

	auto Blocked = [&](int32 Wall, double A, double HalfWidth)
	{
		for (const FVector2D& R : DoorSpans[Wall])
		{
			if (A + HalfWidth > R.X && A - HalfWidth < R.Y)
			{
				return true;
			}
		}
		return false;
	};

	// 트러스: 가로보 + (평천장이면 두 기둥과 이음보, 아니면 가운데 기둥) + 경사 천장 밑 서까래. 박공벽 자리는 뺀다.
	TArray<double> TrussU;
	if (Settings->TrussEvery > 0)
	{
		const double Step = Settings->TrussEvery * Cell;
		for (double U = Step; U < Len - 1.0; U += Step)
		{
			TrussU.Add(U);
		}
	}
	const FVector RafterScale(Run / UE_DOUBLE_INV_SQRT_2 / Cell, 1.0, 1.0);
	auto Vertical = [&](double U, double V, double Z0, double Z1)
	{
		Add(Settings->RafterMesh, U, V, Z0, 0.0, FVector((Z1 - Z0) / Cell, 1.0, 1.0), 90.0);
	};
	for (const double U : TrussU)
	{
		for (int32 c = 0; c < S; ++c)
		{
			Add(Settings->TieBeamMesh, U, c * Cell, TieBeamZ, 90.0);
		}
		const double PostBottom = TieBeamZ + BeamHalf;
		if (bFlat)
		{
			for (int32 c = 0; c * Cell < Flat1 - Flat0 - 1.0; ++c)
			{
				Add(Settings->TieBeamMesh, U, Flat0 + c * Cell, CeilingZ - BeamHalf, 90.0,
					FVector(FMath::Min(Cell, Flat1 - Flat0 - c * Cell) / Cell, 1.0, 1.0));
			}
			Vertical(U, Flat0, PostBottom, CeilingZ - 2.0 * BeamHalf);
			Vertical(U, Flat1, PostBottom, CeilingZ - 2.0 * BeamHalf);
		}
		else
		{
			Vertical(U, Half, PostBottom, RoofBase + Half - 20.0);
		}
		for (int32 k = 0; k < S && (!bFlat || (k + 1) * Run <= Rise + 1.0); ++k)
		{
			const double Z = RoofBase + k * Run - RafterInset;
			Add(Settings->RafterMesh, U, k * Run + RafterInset, Z, 90.0, RafterScale, 45.0);
			Add(Settings->RafterMesh, U, Span - k * Run - RafterInset, Z, -90.0, RafterScale, 45.0);
		}
	}
	// 평천장 장선: 트러스 사이 칸 경계마다 테두리 보 사이를 가로지른다.
	if (bFlat)
	{
		for (int32 i = 1; i < L; ++i)
		{
			const double U = i * Cell;
			if (!TrussU.ContainsByPredicate([U](double T) { return FMath::IsNearlyEqual(T, U, 1.0); }))
			{
				Add(Settings->RafterMesh, U, Flat0, CeilingZ - BeamHalf, 90.0, FVector((Flat1 - Flat0) / Cell, 1.0, 1.0));
			}
		}
	}

	if (Settings->bInteriorDecor)
	{
		const FVector BraceScale(127.0 / Cell, 1.0, 1.0);
		const double PostV = WallFace + BeamHalf;
		int32 BannerIndex = 0;
		auto NextBanner = [&]() -> const TSoftObjectPtr<UStaticMesh>&
		{
			return Settings->WallBannerMeshes[BannerIndex++ % Settings->WallBannerMeshes.Num()];
		};

		for (const double U : TrussU)
		{
			// 가로보 밑 랜턴 두 개(사슬 두 마디).
			for (const double V : { Half - Span * 0.25, Half + Span * 0.25 })
			{
				double Z = TieBeamZ - BeamHalf;
				for (int32 n = 0; n < ChainLinks; ++n, Z -= ChainLength)
				{
					Add(Settings->ChainMesh, U, V, Z, 0.0);
				}
				Add(Settings->HangingLanternMesh, U, V, Z - LanternHeight, 0.0);
				AddLight(U, V, Z - LanternHeight * 0.5);
			}
			// 긴 벽 기둥 + 45도 버팀대, 기둥에 벽 등.
			for (int32 Wall = 0; Wall < 2; ++Wall)
			{
				if (Blocked(Wall, U, 40.0))
				{
					continue;
				}
				const double V = Wall == 0 ? PostV : Span - PostV;
				const double Dir = Wall == 0 ? 1.0 : -1.0;
				Vertical(U, V, 0.0, TieBeamZ - BeamHalf);
				Add(Settings->RafterMesh, U, V + Dir * BeamHalf, TieBeamZ - BeamHalf - 90.0, Dir > 0 ? 90.0 : -90.0, BraceScale, 45.0);
				Add(Settings->SconceMesh, U, V + Dir * BeamHalf, SconceZ, Dir > 0 ? 90.0 : -90.0);
				AddLight(U, V + Dir * 45.0, SconceZ);
			}
		}

		// 기둥 사이(칸 가운데) 긴 벽에 깃발.
		TArray<double> BayCenters;
		{
			double Prev = 0.0;
			for (const double U : TrussU)
			{
				BayCenters.Add((Prev + U) * 0.5);
				Prev = U;
			}
			BayCenters.Add((Prev + Len) * 0.5);
		}
		if (Settings->WallBannerMeshes.Num() > 0)
		{
			for (const double U : BayCenters)
			{
				for (int32 Wall = 0; Wall < 2; ++Wall)
				{
					if (!Blocked(Wall, U, 60.0))
					{
						Add(NextBanner(), U, Wall == 0 ? WallFace + 2.0 : Span - WallFace - 2.0, WallBannerTop, 0.0);
					}
				}
			}
		}

		// 박공 벽(짧은 변) 안쪽: 양쪽 1/4 지점에 벽 등.
		for (int32 End = 0; End < 2; ++End)
		{
			const double U = End == 0 ? WallFace : Len - WallFace;
			for (const double V : { Span * 0.25, Span * 0.75 })
			{
				if (!Blocked(2 + End, V, 40.0))
				{
					Add(Settings->SconceMesh, U, V, SconceZ, End == 0 ? 0.0 : 180.0);
					AddLight(U + (End == 0 ? 35.0 : -35.0), V, SconceZ);
				}
			}
		}
	}

	FPCGTaggedData& MeshOut = Context->OutputData.TaggedData.Emplace_GetRef();
	MeshOut.Data = MakePointData(Context, Transforms, &Meshes);
	MeshOut.Pin = PCGPinConstants::DefaultOutputLabel;
	FPCGTaggedData& LightOut = Context->OutputData.TaggedData.Emplace_GetRef();
	LightOut.Data = MakePointData(Context, Lights, nullptr);
	LightOut.Pin = LightsPin;
	FPCGTaggedData& DoorOut = Context->OutputData.TaggedData.Emplace_GetRef();
	DoorOut.Data = MakePointData(Context, DoorTransforms, &DoorMeshes);
	DoorOut.Pin = DoorPartsPin;
	return true;
}
