요청: 모든 테스트 단계에 중단 시점을 넣고 싶어. 공통적인 헬퍼 함수로 구현하고 각 테스트에서 호출하도록. 기능은 테스트 단계가 종료되면 게임을 멈추고, 다음 테스트가 시작되면 게임을 재개, 끝나면 다시 멈춤. 목적은 다음 단계로 넘어가 테스트가 오류나는걸 방지하기 위함.
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex
테스트: "C:/Program Files/Epic Games/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" "C:/Projects/Unreal/Asteria/Asteria.uproject" /Game/Map/Main -ExecCmds="Automation RunTests Asteria" -TestExit="Automation Test Queue Empty" -unattended -nosplash -nopause -log
로그: Saved/TestLogs/AsteriaTest.log

[1단계]
Source/Asteria/Test/Base/AsteriaNetworkComponent.h (수정)
SetGamePaused(): 에디터의 PIE 일시정지 기능으로 서버와 클라이언트 월드를 함께 멈추거나 재개하고, 호출 한 줄을 남긴 뒤 서버와 클라이언트 월드의 정지 여부를 각각 확인. 아직 얻지 못한 월드는 확인하지 않음 - UEditorEngine::SetPIEWorldsPaused() 호출, UWorld::IsPaused()로 결과 읽기, LogCheck() 호출 (신규)
ResumeGameUntilTestEnd(): 호출한 테스트의 단계 시작 지점에 게임 재개를 등록하고, 테스트가 실패해도 실행되는 정리 단계에 게임 정지를 등록 - SetGamePaused() 호출 (신규)
Log(): 변경 없음 (재사용: Source/Asteria/Test/Base/AsteriaNetworkComponent.h)
LogCheck(): 변경 없음 (재사용: Source/Asteria/Test/Base/AsteriaNetworkComponent.h)


[2단계]
Source/Asteria/Test/Base/AsteriaNetworkSmokeTest.cpp (수정)
ServerAndClient_LoadCurrentMap: 새 PIE 로드 후 게임 재개, 테스트 종료 시 게임 정지, 시작 때 서버와 클라이언트 모두 진행이고 종료 때 모두 정지 - ResumeGameUntilTestEnd() 호출 (재사용: Source/Asteria/Test/Base/AsteriaNetworkComponent.h) (수정, 정상)
  [서버] Main 맵 로드 성공
  [클라이언트] Main 맵 로드 성공
  [서버] 플레이어 캐릭터 로드 성공
  [클라이언트] 플레이어 캐릭터 로드 성공
  [에디터] 게임 재개
  [서버] 게임 상태 기대 진행 == 게임 상태 실제 {실제 서버 게임 상태} (통과)
  [클라이언트] 게임 상태 기대 진행 == 게임 상태 실제 {실제 클라이언트 게임 상태} (통과)
  [에디터] 게임 정지
  [서버] 게임 상태 기대 정지 == 게임 상태 실제 {실제 서버 게임 상태} (통과)
  [클라이언트] 게임 상태 기대 정지 == 게임 상태 실제 {실제 클라이언트 게임 상태} (통과)

Source/Asteria/Test/Npc/AsteriaNpcSpawnTest.cpp (수정)
ServerAndClient_SpawnOneNpcFromSpawner: 앞선 테스트가 멈춘 PIE를 넘겨받아 테스트 단계 시작 때 게임 재개, 테스트 종료 시 게임 정지, 시작 때 서버와 클라이언트 모두 진행이고 종료 때 모두 정지 - ResumeGameUntilTestEnd() 호출 (재사용: Source/Asteria/Test/Base/AsteriaNetworkComponent.h) (수정, 정상)
  [서버] 로드된 환경 사용
  [클라이언트] 로드된 환경 사용
  [에디터] 게임 재개
  [서버] 게임 상태 기대 진행 == 게임 상태 실제 {실제 서버 게임 상태} (통과)
  [클라이언트] 게임 상태 기대 진행 == 게임 상태 실제 {실제 클라이언트 게임 상태} (통과)
  [서버] Npc 스폰 대기 시작
  [서버] Npc 스폰 대기 완료
  [서버] Npc 기대 1 == Npc 실제 {실제 서버 Npc 수} (통과)
  [클라이언트] Npc 스폰 대기 시작
  [클라이언트] Npc 스폰 대기 완료
  [클라이언트] Npc 기대 1 == Npc 실제 {실제 클라이언트 Npc 수} (통과)
  [에디터] 게임 정지
  [서버] 게임 상태 기대 정지 == 게임 상태 실제 {실제 서버 게임 상태} (통과)
  [클라이언트] 게임 상태 기대 정지 == 게임 상태 실제 {실제 클라이언트 게임 상태} (통과)

Source/Asteria/Test/Npc/AsteriaNpcMoveTest.cpp (수정)
ServerAndClient_NpcMovesToQuestBoard: 앞선 테스트가 멈춘 PIE를 넘겨받아 테스트 단계 시작 때 게임 재개, 테스트 종료 시 게임 정지, 시작 때 서버와 클라이언트 모두 진행이고 종료 때 모두 정지 - ResumeGameUntilTestEnd() 호출 (재사용: Source/Asteria/Test/Base/AsteriaNetworkComponent.h) (수정, 정상)
  [서버] 로드된 환경 사용
  [클라이언트] 로드된 환경 사용
  [에디터] 게임 재개
  [서버] 게임 상태 기대 진행 == 게임 상태 실제 {실제 서버 게임 상태} (통과)
  [클라이언트] 게임 상태 기대 진행 == 게임 상태 실제 {실제 클라이언트 게임 상태} (통과)
  [서버] Npc QuestBoard 위치 추적 시작
  [클라이언트] Npc QuestBoard 위치 추적 시작
  [서버] QuestBoard 거리 기대 {서버 도착 반경} 이하 == QuestBoard 거리 실제 {실제 서버 거리} (통과)
  [클라이언트] QuestBoard 거리 기대 {클라이언트 도착 반경} 이하 == QuestBoard 거리 실제 {실제 클라이언트 거리} (통과)
  [에디터] 게임 정지
  [서버] 게임 상태 기대 정지 == 게임 상태 실제 {실제 서버 게임 상태} (통과)
  [클라이언트] 게임 상태 기대 정지 == 게임 상태 실제 {실제 클라이언트 게임 상태} (통과)
