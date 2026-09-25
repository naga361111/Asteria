요청: 퀘스트 보드에서 Npc가 조건에 맞는 적절한 퀘스트를 고르는지를 검증하는 테스트. 아마 클라이언트에서는 이 작업이 없을꺼야. 서버 권위에서만 테스트 되면 돼.
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex
테스트: "C:/Program Files/Epic Games/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" "C:/Projects/Unreal/Asteria/Asteria.uproject" /Game/Map/Main -ExecCmds="Automation RunTests Asteria" -TestExit="Automation Test Queue Empty" -unattended -nosplash -nopause -log
로그: Saved/TestLogs/AsteriaTest.log

[1단계]
Source/Asteria/Test/Npc/AsteriaNpcSelectQuestTest.cpp (신규)
AsteriaNpcSelectQuest: FAsteriaNetworkComponent를 bLoadEnvironment false로 멤버에 두는 네트워크 테스트. 테스트 경로 Asteria.03_Npc.SelectQuest. 이름의 03은 Npc 이동 테스트(02) 다음, 퀘스트 보드에 도착한 Npc를 이어받도록 붙인 실행 순서 번호. 에디터 빌드이면서 ENABLE_PIE_NETWORK_TEST일 때만 컴파일. 서버 월드만 확인 - NETWORK_TEST_CLASS 사용 (재사용: Engine/Source/Developer/CQTest/Public/Components/PIENetworkComponent.h), FAsteriaNetworkComponent, Log(), LogCheck(), ResumeGameUntilTestEnd() 호출 (재사용: Source/Asteria/Test/Base/AsteriaNetworkComponent.h) (신규)
SelectedQuestId(): 서버 월드의 첫 AAsteriaNpc의 NpcId가 파티에 든 Assignment를 AAsteriaGameState의 QuestService.QuestAssignments에서 상태와 상관없이 찾아 그 QuestId를 돌려줌. Npc, QuestService, Assignment 중 하나라도 없으면 -1 - AAsteriaNpc (재사용: Source/Asteria/NPC/AsteriaNpc.h), AAsteriaGameState (재사용: Source/Asteria/GameState/AsteriaGameState.h), UQuestService, FQuestAssignment (재사용: Source/Asteria/GameState/Components/QuestService.h, Source/Asteria/Common/Quest.h) (신규)
AwaitQuestSelected(): SelectedQuestId()가 -1이 아니면 "[서버] Npc 퀘스트 선택 대기 완료"를 남기고 true. 제한 10초가 지나면 "[서버] Npc 퀘스트 선택 대기 시간 초과 10초"를 남기고 true를 돌려 확인을 진행 - SelectedQuestId(), Log() 호출 (신규)

Server_NpcSelectsQuestWithinRank: 앞선 테스트가 퀘스트 보드에 도착시킨 Npc를 그대로 사용, 게임 재개 후 AwaitQuestSelected()로 서버에서 Npc가 퀘스트를 고를 때까지 대기, 고른 퀘스트의 등급을 QuestService.QuestPull에서 읽어 실행 중에 읽은 Npc 등급(NpcRnk) 이하인지 확인. 고른 퀘스트가 없으면 실제 값 없음으로 실패 - NpcAIController가 실행하는 BT_Npc의 UBTTask_SelectQuest가 FindAvailableQuestId(), AssignQuest()로 고른 결과를 서버 월드에서 읽음 (재사용: Source/Asteria/BTNode/BTTask_SelectQuest.cpp, Source/Asteria/GameState/Components/QuestService.cpp) (신규, 정상, 코드 기준)
  [서버] 로드된 환경 사용
  [클라이언트] 로드된 환경 사용
  [에디터] 게임 재개
  [서버] 게임 상태 기대 진행 == 게임 상태 실제 {실제 서버 게임 상태} (통과)
  [클라이언트] 게임 상태 기대 진행 == 게임 상태 실제 {실제 클라이언트 게임 상태} (통과)
  [서버] Npc 퀘스트 선택 대기 시작
  [서버] Npc 퀘스트 선택 대기 완료
  [서버] 선택한 퀘스트 번호 {실제 퀘스트 번호}
  [서버] 선택한 퀘스트 등급 기대 {실제 Npc 등급} 이하 == 선택한 퀘스트 등급 실제 {실제 퀘스트 등급} (통과)
  [에디터] 게임 정지
  [서버] 게임 상태 기대 정지 == 게임 상태 실제 {실제 서버 게임 상태} (통과)
  [클라이언트] 게임 상태 기대 정지 == 게임 상태 실제 {실제 클라이언트 게임 상태} (통과)
Server_SelectedQuestRejectedForOtherNpc: 앞선 테스트가 퀘스트 보드에 도착시킨 Npc를 그대로 사용, 게임 재개 후 AwaitQuestSelected()로 Npc가 고른 퀘스트를 확보, 서버에서 그 퀘스트를 Npc의 NpcId에 1을 더한 다른 Npc 번호 파티로 AssignQuest() 호출, 결과가 INDEX_NONE이면 실패로 읽어 선택 거부를 확인. 고른 퀘스트가 없으면 호출하지 않고 실제 값 퀘스트 없음으로 실패. 호출이 잘못 성공해 Assignment가 생겼으면 정리 단계에서 그 AssignmentId를 QuestAssignments에서 제거 - UQuestService::AssignQuest()의 이미 집힌 퀘스트 검증 호출 (재사용: Source/Asteria/GameState/Components/QuestService.cpp) (신규, 예외, 코드 기준)
  [서버] 로드된 환경 사용
  [클라이언트] 로드된 환경 사용
  [에디터] 게임 재개
  [서버] 게임 상태 기대 진행 == 게임 상태 실제 {실제 서버 게임 상태} (통과)
  [클라이언트] 게임 상태 기대 진행 == 게임 상태 실제 {실제 클라이언트 게임 상태} (통과)
  [서버] Npc 퀘스트 선택 대기 시작
  [서버] Npc 퀘스트 선택 대기 완료
  [서버] 선택한 퀘스트 번호 {실제 퀘스트 번호}
  [서버] 다른 Npc 같은 퀘스트 선택 시작
  [서버] 다른 Npc 같은 퀘스트 선택 완료
  [서버] 다른 Npc 선택 결과 기대 실패 == 다른 Npc 선택 결과 실제 {실제 결과} (통과)
  [에디터] 게임 정지
  [서버] 게임 상태 기대 정지 == 게임 상태 실제 {실제 서버 게임 상태} (통과)
  [클라이언트] 게임 상태 기대 정지 == 게임 상태 실제 {실제 클라이언트 게임 상태} (통과)
