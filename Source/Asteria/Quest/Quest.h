#pragma once

#include "CoreMinimal.h"
#include "Quest.generated.h"

USTRUCT()
struct FQuest
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	int32 QuestId;
	
	UPROPERTY(VisibleAnywhere)
	bool bIsPosted;
};
