요청: 로그를 보면 모든 내용이 주르르 이어져서 나오는데 테스트 별로 공백으로 구분할 필요가 있어. 지금은 LogA\nLogB 이런식인데, 만약 A가 테스트00이고 B가 01이면 LogA\n\nLogB 이런식으로
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex
테스트: "C:/Program Files/Epic Games/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" "C:/Projects/Unreal/Asteria/Asteria.uproject" /Game/Map/Main -ExecCmds="Automation RunTests Asteria" -TestExit="Automation Test Queue Empty" -unattended -nosplash -nopause -log
로그: Saved/TestLogs/AsteriaTest.log

[1단계]
Source/Asteria/Test/Base/AsteriaNetworkComponent.h (수정)
LogTestSeparator(): 두 번째 테스트부터 테스트가 시작될 때 로그 파일에 빈 줄 하나를 기록. 실행의 첫 테스트 앞에는 기록하지 않음 - 생성자가 호출 (신규)
생성자: 초기화 모드가 아닐 때 환경 로드 여부와 상관없이 가장 먼저 LogTestSeparator() 호출 (수정)
Log(): 변경 없음 (재사용: Source/Asteria/Test/Base/AsteriaNetworkComponent.h)
TestSeparator: Asteria 테스트 3개를 한 번에 실행, 로그 파일에서 테스트 사이마다 빈 줄 하나로 구분되고 첫 줄은 빈 줄이 아님 - AsteriaNetworkSmoke, AsteriaNpcSpawn, AsteriaNpcMove 실행 (재사용: Source/Asteria/Test/Base/AsteriaNetworkSmokeTest.cpp, Source/Asteria/Test/Npc/AsteriaNpcSpawnTest.cpp, Source/Asteria/Test/Npc/AsteriaNpcMoveTest.cpp) (수정, 정상)
  [서버] Main 맵 로드 성공
  [클라이언트] Main 맵 로드 성공
  [서버] 플레이어 캐릭터 로드 성공
  [클라이언트] 플레이어 캐릭터 로드 성공

  [서버] 로드된 환경 사용
  [클라이언트] 로드된 환경 사용
  [서버] Npc 스폰 대기 시작
  [서버] Npc 스폰 대기 완료
  [서버] Npc 기대 1 == Npc 실제 {실제 서버 Npc 수} (통과)
  [클라이언트] Npc 스폰 대기 시작
  [클라이언트] Npc 스폰 대기 완료
  [클라이언트] Npc 기대 1 == Npc 실제 {실제 클라이언트 Npc 수} (통과)

  [서버] 로드된 환경 사용
  [클라이언트] 로드된 환경 사용
  [서버] Npc QuestBoard 위치 추적 시작
  [클라이언트] Npc QuestBoard 위치 추적 시작
  [서버] QuestBoard 거리 기대 {서버 도착 반경} 이하 == QuestBoard 거리 실제 {실제 서버 거리} (통과)
  [클라이언트] QuestBoard 거리 기대 {클라이언트 도착 반경} 이하 == QuestBoard 거리 실제 {실제 클라이언트 거리} (통과)
