// Fill out your copyright notice in the Description page of Project Settings.


#include "GuildReputationData.h"

UGuildReputationData::UGuildReputationData()
{
	ReputationByQuestRank = {
		{ERank::F, 1},
		{ERank::E, 2},
		{ERank::D, 4},
		{ERank::C, 8},
		{ERank::B, 16},
		{ERank::A, 32},
		{ERank::S, 64},
	};

	ReputationToReachRank = {
		{ERank::F, 0},
		{ERank::E, 20},
		{ERank::D, 60},
		{ERank::C, 160},
		{ERank::B, 400},
		{ERank::A, 1000},
		{ERank::S, 2500},
	};
}
