// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NpcCharacter.generated.h"

/** NPC의 몸. 지금은 빈 껍데기 — 캡슐/스켈레탈메시/무브먼트는 ACharacter가 준다.
 *  욕구·인벤토리·기억·전투스탯은 나중에 UActorComponent로 여기 붙인다. */
UCLASS()
class ASTERIA_API ANpcCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ANpcCharacter();

protected:
	/** 4대 욕구 게이지. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="NPC")
	TObjectPtr<class UNeedsComponent> Needs;
};
