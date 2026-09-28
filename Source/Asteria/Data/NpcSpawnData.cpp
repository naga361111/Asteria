// Fill out your copyright notice in the Description page of Project Settings.


#include "NpcSpawnData.h"

UNpcSpawnData::UNpcSpawnData()
{
	// F 평균 5분에 한 명. 위 등급은 F 대비 같은 비율로 짧아진다.
	SpawnIntervalMinutesByGuildRank = {
		{ERank::F, 5.f},
		{ERank::E, 4.2f},
		{ERank::D, 3.6f},
		{ERank::C, 2.9f},
		{ERank::B, 2.3f},
		{ERank::A, 1.9f},
		{ERank::S, 1.6f},
	};
}
