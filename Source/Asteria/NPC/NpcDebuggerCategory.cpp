// Copyright Epic Games, Inc. All Rights Reserved.

#include "NpcDebuggerCategory.h"

#if WITH_GAMEPLAY_DEBUGGER

#include "GameplayDebuggerTypes.h"
#include "InputCoreTypes.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "NpcCharacter.h"
#include "NeedsComponent.h"
#include "UtilityBrainComponent.h"
#include "ActionRunnerComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "TemperamentComponent.h"
#include "NpcTemperamentDef.h"
#include "NpcDebugSettings.h"
#include "Engine/DataTable.h"

namespace
{
	constexpr int32 GNumNeeds = 4;

	/** 기질 배율 표시용 욕구 짧은 이름. */
	const TCHAR* NeedName(ENpcNeed Need)
	{
		switch (Need)
		{
		case ENpcNeed::Survival: return TEXT("Survival");
		case ENpcNeed::Safety:   return TEXT("Safety");
		case ENpcNeed::Duty:     return TEXT("Duty");
		case ENpcNeed::Social:   return TEXT("Social");
		default:                 return TEXT("?");
		}
	}

	/** SelectedNeed 인덱스 → NeedsComponent의 해당 int32 필드 포인터. */
	int32* NeedFieldPtr(UNeedsComponent& Needs, int32 Index)
	{
		switch (Index)
		{
		case 0: return &Needs.Survival;
		case 1: return &Needs.Safety;
		case 2: return &Needs.Duty;
		case 3: return &Needs.Social;
		default: return nullptr;
		}
	}
}

FNpcDebuggerCategory::FNpcDebuggerCategory()
{
	// 프레임워크 데이터팩. 싱글플레이라 실제 복제는 안 타지만 규약대로 등록해둔다.
	SetDataPackReplication<FRepData>(&DataPack);

	// 디버거 자체 입력. IMC/IA 아님 — HUD 떠 있을 때만 먹고 게임플레이로 안 샌다.
	BindKeyPress(EKeys::LeftBracket.GetFName(), this, &FNpcDebuggerCategory::OnPrevNeed);
	BindKeyPress(EKeys::RightBracket.GetFName(), this, &FNpcDebuggerCategory::OnNextNeed);
	BindKeyPress(EKeys::Hyphen.GetFName(), this, &FNpcDebuggerCategory::OnDecrement);
	BindKeyPress(EKeys::Equals.GetFName(), this, &FNpcDebuggerCategory::OnIncrement);
}

void FNpcDebuggerCategory::OnPrevNeed()
{
	SelectedNeed = (SelectedNeed + GNumNeeds - 1) % GNumNeeds;
}

void FNpcDebuggerCategory::OnNextNeed()
{
	SelectedNeed = (SelectedNeed + 1) % GNumNeeds;
}

void FNpcDebuggerCategory::OnIncrement()
{
	AdjustNeed(+GetDefault<UNpcDebugSettings>()->NeedStep);
}

void FNpcDebuggerCategory::OnDecrement()
{
	AdjustNeed(-GetDefault<UNpcDebugSettings>()->NeedStep);
}

void FNpcDebuggerCategory::AdjustNeed(int32 Delta)
{
	ANpcCharacter* Npc = CachedNpc.Get();
	if (Npc == nullptr)
	{
		return;
	}
	UNeedsComponent* Needs = Npc->FindComponentByClass<UNeedsComponent>();
	if (Needs == nullptr)
	{
		return;
	}
	if (int32* Field = NeedFieldPtr(*Needs, SelectedNeed))
	{
		*Field = FMath::Clamp(*Field + Delta, 0, 100);
	}
}

TSharedRef<FGameplayDebuggerCategory> FNpcDebuggerCategory::MakeInstance()
{
	return MakeShareable(new FNpcDebuggerCategory());
}

void FNpcDebuggerCategory::FRepData::Serialize(FArchive& Ar)
{
	Ar << bHasNeeds;
	Ar << Survival;
	Ar << Safety;
	Ar << Duty;
	Ar << Social;
	Ar << ChosenAction;
	Ar << CurrentBehavior;
	Ar << TemperamentLines;
}

ANpcCharacter* FNpcDebuggerCategory::FindLookedAtNpc(APlayerController* OwnerPC) const
{
	if (OwnerPC == nullptr)
	{
		return nullptr;
	}

	const UWorld* World = OwnerPC->GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}

	FVector ViewLoc;
	FRotator ViewRot;
	OwnerPC->GetPlayerViewPoint(ViewLoc, ViewRot);

	const UNpcDebugSettings* Settings = GetDefault<UNpcDebugSettings>();
	const FVector TraceEnd = ViewLoc + ViewRot.Vector() * Settings->TraceRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(NpcDebuggerLookAt), /*bTraceComplex=*/false);
	if (const APawn* OwnerPawn = OwnerPC->GetPawn())
	{
		Params.AddIgnoredActor(OwnerPawn);
	}

	// Pawn 프리셋은 Visibility를 무시하므로 오브젝트 타입(Pawn)으로 쿼리한다.
	FHitResult Hit;
	const FCollisionObjectQueryParams ObjParams(ECC_Pawn);
	const bool bHit = World->LineTraceSingleByObjectType(Hit, ViewLoc, TraceEnd, ObjParams, Params);

	// 그리기 시작점만 카메라 기준으로 오프셋(트레이스 판정은 ViewLoc 중앙 유지).
	const FRotationMatrix ViewAxes(ViewRot);
	const FVector DrawStart = ViewLoc
		+ ViewAxes.GetScaledAxis(EAxis::Y) * Settings->DrawOffset.X
		+ ViewAxes.GetScaledAxis(EAxis::Z) * Settings->DrawOffset.Y;

	// 트레이스 경로 디버그. 지속시간 -1 = 단일 프레임, 매 틱 갱신되어 실시간으로 보인다.
	DrawDebugLine(World, DrawStart, bHit ? Hit.ImpactPoint : TraceEnd,
		bHit ? FColor::Green : FColor::Red, /*bPersistent=*/false, /*LifeTime=*/-1.0f);

	return bHit ? Cast<ANpcCharacter>(Hit.GetActor()) : nullptr;
}

void FNpcDebuggerCategory::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	DataPack = FRepData();

	// 바라보는 NPC를 매 수집마다 다시 찾는다 → 카메라를 옮기면 재선택 없이 대상이 따라온다.
	ANpcCharacter* Npc = FindLookedAtNpc(OwnerPC);
	if (Npc == nullptr)
	{
		// 트레이스가 빗나가면 프레임워크 선택(아웃라이너/' latch)으로 폴백.
		Npc = Cast<ANpcCharacter>(DebugActor);
	}

	// 입력 핸들러가 조작할 대상으로 캐시(null 가능).
	CachedNpc = Npc;

	if (Npc == nullptr)
	{
		return;
	}

	if (const UNeedsComponent* NeedsComp = Npc->FindComponentByClass<UNeedsComponent>())
	{
		DataPack.bHasNeeds = true;
		DataPack.Survival = NeedsComp->Survival;
		DataPack.Safety = NeedsComp->Safety;
		DataPack.Duty = NeedsComp->Duty;
		DataPack.Social = NeedsComp->Social;
	}

	// 선택된 행동은 Controller의 브레인이 들고 있다.
	if (const AController* Ctrl = Npc->GetController())
	{
		if (const UUtilityBrainComponent* Brain = Ctrl->FindComponentByClass<UUtilityBrainComponent>())
		{
			DataPack.ChosenAction = Brain->ChosenAction;
		}

		// 지금 실행 중인 원자 BT + 진행(i/n). 비면 Idle.
		if (const UActionRunnerComponent* Runner = Ctrl->FindComponentByClass<UActionRunnerComponent>())
		{
			if (const UBehaviorTree* BT = Runner->GetCurrentBehavior())
			{
				DataPack.CurrentBehavior = FString::Printf(TEXT("%s (%d/%d)"),
					*BT->GetName(), Runner->GetStepIndex() + 1, Runner->GetStepCount());
			}
			else
			{
				DataPack.CurrentBehavior = TEXT("(idle)");
			}
		}
	}

	// 가진 기질 목록 + 각 기질이 주는 욕구별 배율(행에 작성된 값).
	if (const UTemperamentComponent* Temp = Npc->FindComponentByClass<UTemperamentComponent>())
	{
		for (const FName& Name : Temp->Temperaments)
		{
			FString Line = Name.ToString();
			if (Temp->Table != nullptr)
			{
				if (const FNpcTemperamentDef* Row = Temp->Table->FindRow<FNpcTemperamentDef>(Name, TEXT("NpcDebugger")))
				{
					for (const TPair<ENpcNeed, float>& M : Row->Multipliers)
					{
						Line += FString::Printf(TEXT("  %s x%g"), NeedName(M.Key), M.Value);
					}
				}
			}
			DataPack.TemperamentLines.Add(Line);
		}
	}
}

void FNpcDebuggerCategory::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	if (!DataPack.bHasNeeds)
	{
		CanvasContext.Printf(TEXT("{yellow}No NPC targeted (or no NeedsComponent)."));
		return;
	}

	// 오른쪽에 기질 열을 겹쳐 그리기 위해 두 열의 상단 Y를 맞춘다.
	const float TopY = CanvasContext.CursorY;
	const float ColX = CanvasContext.DefaultX + 220.0f;
	const float LineH = CanvasContext.GetLineHeight();

	// 오른쪽 열: 가진 기질 목록(PrintAt은 메인 커서를 안 옮겨 왼쪽 흐름과 독립).
	CanvasContext.PrintAt(ColX, TopY, FColor::Green, TEXT("Temperaments"));
	if (DataPack.TemperamentLines.Num() == 0)
	{
		CanvasContext.PrintAt(ColX, TopY + LineH, FColor::White, TEXT("(none)"));
	}
	else
	{
		for (int32 i = 0; i < DataPack.TemperamentLines.Num(); ++i)
		{
			CanvasContext.PrintAt(ColX, TopY + LineH * (i + 1), FColor::White, DataPack.TemperamentLines[i]);
		}
	}

	// 화면 텍스트는 디버그 폰트 글리프 문제를 피하려 ASCII로 둔다.
	CanvasContext.Printf(TEXT("{green}Needs  {grey}([ ] select   - = adjust)"));

	static const TCHAR* Labels[GNumNeeds] =
	{
		TEXT("Survival"), TEXT("Safety  "), TEXT("Duty    "), TEXT("Social  ")
	};
	const int32 Values[GNumNeeds] =
	{
		DataPack.Survival, DataPack.Safety, DataPack.Duty, DataPack.Social
	};
	for (int32 i = 0; i < GNumNeeds; ++i)
	{
		if (i == SelectedNeed)
		{
			CanvasContext.Printf(TEXT("{yellow}> %s : %d"), Labels[i], Values[i]);
		}
		else
		{
			CanvasContext.Printf(TEXT("  {white}%s : %d"), Labels[i], Values[i]);
		}
	}
	CanvasContext.Printf(TEXT("{green}Chosen : {white}%s"), *DataPack.ChosenAction.ToString());
	CanvasContext.Printf(TEXT("{green}Action BT : {white}%s"), *DataPack.CurrentBehavior);
}

#endif // WITH_GAMEPLAY_DEBUGGER
