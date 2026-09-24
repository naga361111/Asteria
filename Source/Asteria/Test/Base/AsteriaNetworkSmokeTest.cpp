// Fill out your copyright notice in the Description page of Project Settings.

// 게임 빌드에는 CQTest가 없으므로 에디터 빌드에서만 컴파일한다.
#if WITH_EDITOR

#include "Test/Base/AsteriaNetworkComponent.h"

#if ENABLE_PIE_NETWORK_TEST

// 공통 기반이 서버와 클라이언트를 띄우고 현재 맵을 싣는지 로그로 확인하는 테스트.
NETWORK_TEST_CLASS(AsteriaNetworkSmoke, "Asteria.00_Base.Network")
{
	FAsteriaNetworkComponent Network{ TestRunner, TestCommandBuilder, bInitializing };

	TEST_METHOD(ServerAndClient_LoadCurrentMap)
	{
	}
};

#endif // ENABLE_PIE_NETWORK_TEST

#endif // WITH_EDITOR
