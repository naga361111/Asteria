요청: 테스트 01 단계에서 서버와 클라이언트 각각에 Npc가 스폰된걸 검증했어. 이제 해당 Npc가 QuestBoard로 성공적으로 이동하는지를 검증해야 해.
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex
테스트: "C:/Program Files/Epic Games/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" "C:/Projects/Unreal/Asteria/Asteria.uproject" /Game/Map/Main -ExecCmds="Automation RunTests Asteria" -TestExit="Automation Test Queue Empty" -unattended -nosplash -nopause -log
로그: Saved/TestLogs/AsteriaTest.log

[1단계]
Source/Asteria/Test/Npc/AsteriaNpcMoveTest.cpp (신규)
AsteriaNpcMove: FAsteriaNetworkComponent를 bLoadEnvironment false로 멤버에 두는 네트워크 테스트. 테스트 경로 Asteria.02_Npc.MoveToQuestBoard. 이름의 02는 Npc 스폰 테스트(01) 다음에 실행되도록 붙인 실행 순서 번호. 에디터 빌드이면서 ENABLE_PIE_NETWORK_TEST일 때만 컴파일 - NETWORK_TEST_CLASS 사용 (재사용: Engine/Source/Developer/CQTest/Public/Components/PIENetworkComponent.h), FAsteriaNetworkComponent, Log(), LogCheck() 호출 (재사용: Source/Asteria/Test/Base/AsteriaNetworkComponent.h) (신규)
ReadQuestBoardRadius(): BT_Npc 에셋(/Game/Blueprints/Behavior/BT_Npc)을 불러 루트부터 하위 노드를 따라가 TargetType이 QuestBoard인 UBTTask_MoveToApproachPoint 노드를 찾고, 그 노드에 저장된 AcceptableRadius 값을 돌려줌. 두 속성이 protected라 리플렉션으로 읽기만 함. 노드가 없으면 -1 - BT_Npc 에셋 (재사용: Content/Blueprints/Behavior/BT_Npc.uasset), UBTTask_MoveToApproachPoint (재사용: Source/Asteria/BTNode/BTTask_MoveToApproachPoint.h) (신규)
DistanceToQuestBoard(): 주어진 월드의 첫 AAsteriaNpc와 그 월드에 배치된 PointType이 QuestBoard인 AApproachPointActor들 사이의 가장 짧은 수평 거리(높이 제외)를 정수로 돌려줌. Npc나 접근 지점이 없으면 -1 - AAsteriaNpc (재사용: Source/Asteria/NPC/AsteriaNpc.h), AApproachPointActor (재사용: Source/Asteria/Actors/ApproachPointActor.h) (신규)
ServerAndClient_NpcMovesToQuestBoard: 앞선 테스트가 로드한 환경을 그대로 사용, 서버와 클라이언트가 각자 ReadQuestBoardRadius()로 도착 반경을 읽고 각자의 월드를 저장한 뒤, 한 대기 단계에서 매 프레임 서버 월드와 클라이언트 월드를 각각 따로 확인해 그 월드의 DistanceToQuestBoard()가 그 쪽이 읽은 도착 반경 이하가 된 첫 순간의 거리를 기록. 제한 30초, 초과 시 도착하지 못한 쪽마다 "[서버] Npc QuestBoard 도착 대기 시간 초과 30초" 형식으로 한 줄 기록하고 그 순간의 거리로 확인 진행. 서버와 클라이언트 각각 기록된 거리가 자기가 읽은 도착 반경 이하 - NpcAIController가 실행하는 BT_Npc의 UBTTask_MoveToApproachPoint가 이동시킨 결과를 서버 월드에서 읽음 (재사용: Source/Asteria/BTNode/BTTask_MoveToApproachPoint.cpp), 클라이언트 월드에서는 AAsteriaNpc의 이동 리플리케이션 결과를 읽음 (재사용: Source/Asteria/NPC/AsteriaNpc.cpp) (신규, 정상)
  [서버] 로드된 환경 사용
  [클라이언트] 로드된 환경 사용
  [서버] BT_Npc QuestBoard 이동 노드 도착 반경 {실제 서버 반경}
  [클라이언트] BT_Npc QuestBoard 이동 노드 도착 반경 {실제 클라이언트 반경}
  [서버] Npc QuestBoard 이동 대기 시작
  [클라이언트] Npc QuestBoard 이동 대기 시작
  [서버] Npc QuestBoard 도착
  [클라이언트] Npc QuestBoard 도착
  [서버] QuestBoard 거리 기대 {실제 서버 반경} 이하 == QuestBoard 거리 실제 {실제 서버 거리} (통과)
  [클라이언트] QuestBoard 거리 기대 {실제 클라이언트 반경} 이하 == QuestBoard 거리 실제 {실제 클라이언트 거리} (통과)
