// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#if WITH_GAMEPLAY_DEBUGGER

#include "CoreMinimal.h"
#include "GameplayDebuggerCategory.h"

class ANpcCharacter;

/** "Asteria.NPC" 디버거 카테고리 — 크로스헤어가 가리키는 NPC의 내부 수치를 화면에 뿌린다.
 *  게임플레이 클래스/컴포넌트는 수정하지 않고 밖에서 읽기만 한다(관찰 전용). */
class FNpcDebuggerCategory : public FGameplayDebuggerCategory
{
public:
	FNpcDebuggerCategory();

	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
	virtual void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;

private:
	/** 플레이어 시점에서 정면 트레이스로 바라보는 NPC를 찾는다. 못 찾으면 nullptr. */
	const ANpcCharacter* FindLookedAtNpc(APlayerController* OwnerPC) const;

	/** CollectData가 채우고 DrawData가 읽는다. 새 컴포넌트가 붙으면 여기에 필드를 늘린다. */
	struct FRepData
	{
		bool bHasNeeds = false;
		int32 Survival = 0;
		int32 Safety = 0;
		int32 Duty = 0;
		int32 Social = 0;
		FName ChosenAction;

		void Serialize(FArchive& Ar);
	};
	FRepData DataPack;
};

#endif // WITH_GAMEPLAY_DEBUGGER
