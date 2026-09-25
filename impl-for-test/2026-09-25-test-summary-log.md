요청: 테스트 코드가 작성하는 로그를 2개로 분리해줘. 현재의 상세 로그는 유지하면서, 그냥 각 단계별로 성공/실패 한 줄만 나오는 단순하게 확인할 수 있는 로그도 필요해
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex
테스트: "C:/Program Files/Epic Games/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" "C:/Projects/Unreal/Asteria/Asteria.uproject" /Game/Map/Main -ExecCmds="Automation RunTests Asteria" -TestExit="Automation Test Queue Empty" -unattended -nosplash -nopause -log
로그: Saved/TestLogs/AsteriaTest.log (상세), Saved/TestLogs/AsteriaTestSummary.log (요약)

[1단계]
Source/Asteria/Test/Base/AsteriaNetworkComponent.h (수정)
LogSummary(): 요약 한 줄을 언리얼 로그와 Saved/TestLogs/AsteriaTestSummary.log에만 기록. 상세 로그 파일에는 쓰지 않음. 파일은 실행마다 새로 씀 - UE_LOG 호출, FFileHelper::SaveStringToFile() 호출 (신규)
LogTestResult(): 테스트 하나가 끝날 때 전체 테스트 이름과 통과 또는 실패를 한 줄로 요약 로그에 기록. 테스트에 오류가 하나라도 있으면 실패 - FAutomationTestBase::GetTestFullName(), FAutomationTestBase::HasAnyErrors() 호출, LogSummary() 호출 (신규)
생성자: 초기화 모드가 아닐 때 환경 로드 여부와 상관없이 LogTestResult()를 정리 단계에 등록. 테스트가 실패해도 실행되고, 게임 정지 확인과 퀘스트 정리 등 다른 정리 단계보다 뒤에 실행되어 그 결과까지 반영 (수정)
Log(): 변경 없음. 상세 로그 형식과 내용 그대로 (재사용: Source/Asteria/Test/Base/AsteriaNetworkComponent.h)
LogCheck(): 변경 없음 (재사용: Source/Asteria/Test/Base/AsteriaNetworkComponent.h)
LogTestSeparator(): 변경 없음 (재사용: Source/Asteria/Test/Base/AsteriaNetworkComponent.h)

SummaryAllPass: Asteria 테스트 전체를 한 번에 실행, 요약 로그에 테스트마다 한 줄씩 실행 순서대로 통과가 기록되고 상세 로그는 이전과 같은 형식으로 그대로 기록됨 - AsteriaNetworkSmoke, AsteriaNpcSpawn, AsteriaNpcMove, AsteriaNpcSelectQuest 실행 (재사용: Source/Asteria/Test/Base/AsteriaNetworkSmokeTest.cpp, Source/Asteria/Test/Npc/AsteriaNpcSpawnTest.cpp, Source/Asteria/Test/Npc/AsteriaNpcMoveTest.cpp, Source/Asteria/Test/Npc/AsteriaNpcSelectQuestTest.cpp) (수정, 정상)
  [에디터] Asteria.00_Base.Network.ServerAndClient_LoadCurrentMap 통과
  [에디터] Asteria.01_Npc.Spawn.ServerAndClient_SpawnOneNpcFromSpawner 통과
  [에디터] Asteria.02_Npc.MoveToQuestBoard.ServerAndClient_NpcMovesToQuestBoard 통과
  [에디터] Asteria.03_Npc.SelectQuest.Server_NpcSelectsQuestWithinRank 통과
  [에디터] Asteria.03_Npc.SelectQuest.Server_SelectedQuestRejectedForOtherNpc 통과
SummaryFailWithoutPie: 앞선 테스트 없이 Asteria.01_Npc.Spawn 하나만 실행, 띄워진 PIE가 없어 월드 대기가 시간 초과로 실패하고 요약 로그에 실패 한 줄이 기록됨. 테스트 명령의 RunTests 대상만 Asteria.01_Npc.Spawn으로 바꿔 실행 - AsteriaNpcSpawn 실행 (재사용: Source/Asteria/Test/Npc/AsteriaNpcSpawnTest.cpp), FAsteriaNetworkComponent의 Set Worlds 대기 (재사용: Source/Asteria/Test/Base/AsteriaNetworkComponent.h) (수정, 예외, 코드 기준)
  [에디터] Asteria.01_Npc.Spawn.ServerAndClient_SpawnOneNpcFromSpawner 실패
