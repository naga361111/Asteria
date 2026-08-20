// Copyright Epic Games, Inc. All Rights Reserved.

#include "NpcDebuggerCategory.h"

#if WITH_GAMEPLAY_DEBUGGER

#include "GameplayDebuggerTypes.h"
#include "NpcCharacter.h"
#include "NeedsComponent.h"

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

void FNpcDebuggerCategory::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	DataPack = FRepData();

	const ANpcCharacter* Npc = Cast<ANpcCharacter>(DebugActor);
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
