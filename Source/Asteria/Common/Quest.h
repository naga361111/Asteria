#pragma once

#include "CoreMinimal.h"
#include "Quest.generated.h"

UENUM()
enum class EQuest: uint8
{
	Generated,
	Selected,
	Accepted,
	Cleared
};

USTRUCT()
struct FQuest
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	int32 QuestId;
	
	UPROPERTY(VisibleAnywhere)
	EQuest QuestType;
};
