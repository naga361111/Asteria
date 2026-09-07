#pragma once

#include "CoreMinimal.h"
#include "Common/Rank.h"
#include "Quest.generated.h"

UENUM()
enum class EQuestType : uint8
{
	Generated, // 시스템에 의해 생성
	Posted, // Npc가 보는 게시판에 게시
	Selected, // Npc가 선턱
	Accepted, // 유저에 의해 퀘스트 수주 확정
	Cleared, // 유저에 의해 퀘스트 완료 확정
};

USTRUCT()
struct FQuest
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	int32 QuestId;
	
	UPROPERTY(VisibleAnywhere)
	EQuestType QuestType;
	
	UPROPERTY(VisibleAnywhere)
	ERank RecommendedRank;
};
