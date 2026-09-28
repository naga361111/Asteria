// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Common/Rank.h"
#include "NpcSpawnService.generated.h"

class AAsteriaNpc;
class AApproachPointActor;
class UNpcSpawnData;

/**
 * NPC 방문(주기 스폰)을 결정한다. 언제·몇 명·어떤 등급으로 올지는 게임 전체 규칙이라 여기서 한 벌로 돈다.
 * 어디서 나타날지는 맵에 배치한 Spawn 타입 접근 지점 중 무작위.
 * 전부 서버 전용 — NPC는 스폰 후 리플리케이션으로 클라에 나타난다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ASTERIA_API UNpcSpawnService : public UActorComponent
{
	GENERATED_BODY()

public:
	UNpcSpawnService();

	// 스폰할 NPC 클래스. BP_GameState의 컴포넌트 기본값에서 지정한다.
	UPROPERTY(EditDefaultsOnly, Category="Npc")
	TSubclassOf<AAsteriaNpc> NpcClass;

	// 길드 등급별 방문 간격 표. BP_GameState의 컴포넌트 기본값에서 지정한다.
	UPROPERTY(EditDefaultsOnly, Category="Npc")
	TObjectPtr<UNpcSpawnData> SpawnData;

protected:
	// 서버에서 스폰 확인 타이머를 시작한다.
	virtual void BeginPlay() override;

private:
	// --- 주기 스폰: 전부 서버 전용, 복제 안 함 ---

	FTimerHandle SpawnTimer;
	// 직전 확인 시각(게임 분). 확인 사이 흐른 게임 분만큼 확률을 적용하는 기준.
	int32 LastCheckMinute = 0;
	// 스폰마다 발급하는 고유 NpcId. 0은 에디터 기본값이라 1부터.
	int32 NextNpcId = 1;

	// SpawnTimer가 주기적으로 호출. 길드 등급별 평균 간격으로 확률적으로 스폰한다.
	void TickSpawnClock();

	// Point 위치·회전에 NPC 하나를 스폰하고 고유 NpcId와 등급을 부여한다.
	void SpawnNpc(const AApproachPointActor& Point, ERank Rank);
};
