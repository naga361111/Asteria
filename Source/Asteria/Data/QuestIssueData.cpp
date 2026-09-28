// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestIssueData.h"

UQuestIssueData::UQuestIssueData()
{
	// NPC 한 명이 퀘스트 하나를 집고 떠나므로 공급을 방문과 1:1로 맞춘다.
	// 간격 = 평균 묶음 크기(3.5) × NpcSpawnData의 등급별 방문 간격. 흔들림은 간격의 1/6,
	// 제한 시간은 간격의 4/3~8/3배(아무도 안 집으면 게시판에 약 두 묶음이 쌓이는 정도). 전부 분 단위 반올림.
	auto Make = [](int32 Period, int32 Jitter, int32 MinLifetime, int32 MaxLifetime)
	{
		FQuestIssueSettings Settings;
		Settings.BatchPeriodMinutes = Period;
		Settings.BatchJitterMinutes = Jitter;
		Settings.MinQuestLifetimeMinutes = MinLifetime;
		Settings.MaxQuestLifetimeMinutes = MaxLifetime;
		return Settings;
	};

	// 순서: 간격, 흔들림, 제한 최소, 제한 최대
	IssueSettingsByGuildRank = {
		{ERank::F, Make(18, 3, 24, 48)},
		{ERank::E, Make(15, 3, 20, 40)},
		{ERank::D, Make(13, 2, 17, 35)},
		{ERank::C, Make(10, 2, 13, 27)},
		{ERank::B, Make(8, 1, 11, 21)},
		{ERank::A, Make(7, 1, 9, 19)},
		{ERank::S, Make(6, 1, 8, 16)},
	};
}
