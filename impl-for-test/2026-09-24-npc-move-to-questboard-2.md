요청: Npc의 특정 지점 도착 테스트를 서버와 클라이언트 각각에서 개별적으로 위치를 추적해서 반경 50이내에 도착했는지로 검사하도록 기존에 존재하는 테스트 로직을 수정
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex
테스트: "C:/Program Files/Epic Games/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" "C:/Projects/Unreal/Asteria/Asteria.uproject" /Game/Map/Main -ExecCmds="Automation RunTests Asteria" -TestExit="Automation Test Queue Empty" -unattended -nosplash -nopause -log
로그: Saved/TestLogs/AsteriaTest.log

[1단계]
Source/Asteria/Test/Npc/AsteriaNpcMoveTest.cpp (수정)
AsteriaNpcMove: FAsteriaNetworkComponent를 bLoadEnvironment false로 멤버에 두는 네트워크 테스트. 테스트 경로 Asteria.02_Npc.MoveToQuestBoard. 이름의 02는 Npc 스폰 테스트(01) 다음에 실행되도록 붙인 실행 순서 번호. 에디터 빌드이면서 ENABLE_PIE_NETWORK_TEST일 때만 컴파일 - NETWORK_TEST_CLASS 사용 (재사용: Engine/Source/Developer/CQTest/Public/Components/PIENetworkComponent.h), FAsteriaNetworkComponent, Log(), LogCheck() 호출 (재사용: Source/Asteria/Test/Base/AsteriaNetworkComponent.h) (신규)
DistanceToQuestBoard(): 주어진 월드의 첫 AAsteriaNpc와 그 월드에 배치된 PointType이 QuestBoard인 AApproachPointActor들 사이의 가장 짧은 수평 거리(높이 제외)를 정수로 돌려줌. Npc나 접근 지점이 없으면 -1 - AAsteriaNpc (재사용: Source/Asteria/NPC/AsteriaNpc.h), AApproachPointActor (재사용: Source/Asteria/Actors/ApproachPointActor.h) (신규)
ArriveRadius(): 주어진 월드의 첫 AAsteriaNpc 캡슐 반경을 실행 중에 읽어 도착 반경 50 + 캡슐 반경 × 1.1을 정수로 돌려줌. 1.1은 엔진 UPathFollowingComponent의 MinAgentRadiusPct 기본값으로, protected이고 리플렉션 대상이 아니라 읽을 수 없어 적어 둠. MoveToActor 기본 호출(bStopOnOverlap true)이 몸통 가장자리 기준으로 멈추는 실제 판정과 맞춤. Npc가 없으면 -1 - AAsteriaNpc 캡슐 컴포넌트 (재사용: Source/Asteria/NPC/AsteriaNpc.h) (신규)
ServerAndClient_NpcMovesToQuestBoard: 앞선 테스트가 로드한 환경을 그대로 사용, 서버 월드와 클라이언트 월드를 각각 저장하고 각 월드에서 ArriveRadius()로 도착 반경을 읽은 뒤, 한 대기 단계에서 매 프레임 두 월드의 DistanceToQuestBoard()를 서로 따로 읽어 월드마다 추적 중 가장 가까웠던 거리를 갱신하고, 그 거리가 그 월드의 도착 반경 이하가 되면 그 월드는 도착으로 추적을 끝냄. 두 월드가 모두 도착하면 대기 종료. 제한 30초, 초과 시 도착하지 못한 쪽마다 "[서버] Npc QuestBoard 도착 대기 시간 초과 30초" 형식으로 한 줄 기록하고 확인 진행. 서버와 클라이언트 각각 추적 중 가장 가까웠던 거리가 자기 월드의 도착 반경 이하 - NpcAIController가 실행하는 BT_Npc의 UBTTask_MoveToApproachPoint가 이동시킨 결과를 서버 월드에서 읽음 (재사용: Source/Asteria/BTNode/BTTask_MoveToApproachPoint.cpp), 클라이언트 월드에서는 AAsteriaNpc의 이동 리플리케이션 결과를 읽음 (재사용: Source/Asteria/NPC/AsteriaNpc.cpp) (수정, 정상, 코드 기준)
  [서버] 로드된 환경 사용
  [클라이언트] 로드된 환경 사용
  [서버] Npc QuestBoard 위치 추적 시작
  [클라이언트] Npc QuestBoard 위치 추적 시작
  [서버] QuestBoard 거리 기대 {실제 서버 도착 반경} 이하 == QuestBoard 거리 실제 {실제 서버 최소 거리} (통과)
  [클라이언트] QuestBoard 거리 기대 {실제 클라이언트 도착 반경} 이하 == QuestBoard 거리 실제 {실제 클라이언트 최소 거리} (통과)
