// Copyright Epic Games, Inc. All Rights Reserved.

#include "NpcDebuggerCategory.h"

#if WITH_GAMEPLAY_DEBUGGER

#include "GameplayDebuggerTypes.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "NpcCharacter.h"
#include "NeedsComponent.h"
#include "NpcDebugSettings.h"

FNpcDebuggerCategory::FNpcDebuggerCategory()
{
	// 프레임워크 데이터팩. 싱글플레이라 실제 복제는 안 타지만 규약대로 등록해둔다.
	SetDataPackReplication<FRepData>(&DataPack);
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
}

const ANpcCharacter* FNpcDebuggerCategory::FindLookedAtNpc(APlayerController* OwnerPC) const
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
	const ANpcCharacter* Npc = FindLookedAtNpc(OwnerPC);
	if (Npc == nullptr)
	{
		// 트레이스가 빗나가면 프레임워크 선택(아웃라이너/' latch)으로 폴백.
		Npc = Cast<ANpcCharacter>(DebugActor);
	}
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
}

void FNpcDebuggerCategory::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	if (!DataPack.bHasNeeds)
	{
		CanvasContext.Printf(TEXT("{yellow}No NPC targeted (or no NeedsComponent)."));
		return;
	}

	// 화면 텍스트는 디버그 폰트 글리프 문제를 피하려 ASCII로 둔다.
	CanvasContext.Printf(TEXT("{green}Needs"));
	CanvasContext.Printf(TEXT("  Survival : {white}%d"), DataPack.Survival);
	CanvasContext.Printf(TEXT("  Safety   : {white}%d"), DataPack.Safety);
	CanvasContext.Printf(TEXT("  Duty     : {white}%d"), DataPack.Duty);
	CanvasContext.Printf(TEXT("  Social   : {white}%d"), DataPack.Social);
}

#endif // WITH_GAMEPLAY_DEBUGGER
