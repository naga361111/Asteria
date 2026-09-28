// Fill out your copyright notice in the Description page of Project Settings.


#include "NpcSpawnService.h"

#include "EngineUtils.h"
#include "GameClockService.h"
#include "GuildService.h"
#include "Actors/ApproachPointActor.h"
#include "Data/NpcSpawnData.h"
#include "GameState/AsteriaGameState.h"
#include "NPC/AsteriaNpc.h"

namespace
{
	// 스폰 여부를 확인하는 실제 시간 주기(초).
	constexpr float SpawnCheckIntervalSeconds = 1.f;

	// 등급 추첨은 QuestService의 RollQuestRank와 같은 규칙(복사본). 퀘스트 쪽을 바꾸면 여기도 맞출 것.
	// 길드 등급에서 한 단계 내려갈 때마다 곱하는 무게 비율.
	constexpr float LowerRankFalloff = 0.5f;
	// 길드 등급 바로 위 한 단계의 무게. 그보다 위는 나오지 않는다.
	constexpr float UpperRankWeight = 0.3f;

	// 길드 등급 기준 가중 추첨: 동급 1, 아래는 단계마다 LowerRankFalloff 배, 위 한 단계만 UpperRankWeight.
	ERank RollNpcRank(ERank GuildRank)
	{
		const int32 Guild = static_cast<int32>(GuildRank);
		const int32 Top = FMath::Min(Guild + 1, static_cast<int32>(ERank::S)); // S급 초과는 없다

		float Weight[static_cast<int32>(ERank::S) + 1] = {};
		float TotalWeight = 0.f;
		for (int32 R = 0; R <= Top; ++R)
		{
			Weight[R] = R > Guild ? UpperRankWeight : FMath::Pow(LowerRankFalloff, static_cast<float>(Guild - R));
			TotalWeight += Weight[R];
		}

		// PickedRnk를 매 등급마다 갱신해 FP 잔차로 아무것도 안 뽑히는 걸 막는다.
		float Roll = FMath::FRand() * TotalWeight;
		int32 PickedRnk = 0;
		for (int32 R = 0; R <= Top; ++R)
		{
			PickedRnk = R;
			Roll -= Weight[R];
			if (Roll < 0.f)
			{
				break;
			}
		}
		return static_cast<ERank>(PickedRnk);
	}
}

UNpcSpawnService::UNpcSpawnService()
{
	// 복제할 상태가 없다 — 결정은 서버에서만 하고, 결과(NPC)는 NPC 자체의 리플리케이션으로 전달된다.
	SetIsReplicatedByDefault(false);
	PrimaryComponentTick.bCanEverTick = false;
}

void UNpcSpawnService::BeginPlay()
{
	Super::BeginPlay();

	// 서버 권위: 스폰은 호스트에서만. 클라는 리플리케이션으로 받는다.
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	const AAsteriaGameState* GameState = Cast<AAsteriaGameState>(GetOwner());
	if (!GameState || !GameState->GameClockService || !GameState->GuildService
		|| !NpcClass || !SpawnData)
	{
		UE_LOG(LogTemp, Warning, TEXT("NpcSpawnService: no owning GameState, GameClockService, GuildService, NpcClass or SpawnData. NPCs not spawned."));
		return;
	}

	LastCheckMinute = GameState->GameClockService->GetGameMinutes();

	GetWorld()->GetTimerManager().SetTimer(SpawnTimer, this, &UNpcSpawnService::TickSpawnClock,
		SpawnCheckIntervalSeconds, true);
}

void UNpcSpawnService::TickSpawnClock()
{
	const AAsteriaGameState* GameState = Cast<AAsteriaGameState>(GetOwner());
	if (!GameState || !GameState->GameClockService || !GameState->GuildService)
	{
		return;
	}

	const int32 Now = GameState->GameClockService->GetGameMinutes();
	const int32 ElapsedMinutes = Now - LastCheckMinute;
	LastCheckMinute = Now;

	// ponytail: 방문은 길드 등급 표만 본다. 빈 자리·집을 퀘스트 유무는 아직 안 따짐 — 필요해지면 여기에 조건 추가.
	// 평균 간격은 길드 등급 표에서 온다. 표나 항목이 없거나 0 이하면 스폰하지 않는다.
	const ERank GuildRank = GameState->GuildService->GuildRank;
	const float* MeanInterval = SpawnData ? SpawnData->SpawnIntervalMinutesByGuildRank.Find(GuildRank) : nullptr;
	if (!MeanInterval || *MeanInterval <= 0.f)
	{
		return;
	}

	// 흐른 분 동안 한 번이라도 방문할 확률. 확인은 1분마다라 1분에 최대 한 명 — 겹침은 스폰 충돌 보정이 처리한다.
	const float SpawnChance = 1.f - FMath::Exp(-ElapsedMinutes / *MeanInterval);
	if (FMath::FRand() >= SpawnChance)
	{
		return;
	}

	// 스폰 지점이 여럿이면 무작위. 조건이 아니라 위치가 없어 못 놓는 경우라 경고만 남긴다.
	TArray<AApproachPointActor*> SpawnPoints;
	for (TActorIterator<AApproachPointActor> It(GetWorld()); It; ++It)
	{
		if (It->PointType == EApproachPointType::Spawn)
		{
			SpawnPoints.Add(*It);
		}
	}
	if (SpawnPoints.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("NpcSpawnService: no Spawn approach point. NPC not spawned."));
		return;
	}

	SpawnNpc(*SpawnPoints[FMath::RandRange(0, SpawnPoints.Num() - 1)], RollNpcRank(GuildRank));
}

void UNpcSpawnService::SpawnNpc(const AApproachPointActor& Point, ERank Rank)
{
	if (!NpcClass)
	{
		return;
	}

	// 지연 스폰: BeginPlay·빙의(BT 시작) 전에 NpcId·등급을 넣어야 BT가 처음부터 올바른 값으로 돈다.
	const FTransform SpawnTransform(Point.GetActorRotation(), Point.GetActorLocation());
	AAsteriaNpc* Npc = GetWorld()->SpawnActorDeferred<AAsteriaNpc>(NpcClass, SpawnTransform, nullptr, nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	if (!Npc)
	{
		return;
	}

	Npc->NpcId = NextNpcId++;
	Npc->NpcRnk = Rank;
	Npc->FinishSpawning(SpawnTransform);

	if (!Npc->GetController())
	{
		// 런타임 스폰은 auto-possess가 안 걸리는 경우가 있어 명시적으로 AI 컨트롤러 부여
		Npc->SpawnDefaultController();
	}
}
