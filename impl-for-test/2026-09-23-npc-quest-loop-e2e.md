요청: 서버 전용으로, Npc의 루프를 검증해야 해. Npc는 런타임에 퀘스트 보드 앞으로 가서 조건에 맞는 퀘스트 수주-> 카운터로 가서 수주한 퀘스트 제출 후 Assignment의 플레이어 컨펌 대기-> 플레이어가 컨펌하면 Dungeon으로 이동해 퀘스트 수행-> 완료 후 카운터로 돌아가 Assignment 제출 후 컨펌 대기-> 플레이어가 컨펌해주면 정산(이 과정에서 Npc나 플레이어 측에서 변하는건 없음)-> 정산이 완료되면 Npc는 퀘스트 보드로 가서 다음 퀘스트 수주. 이 일련의 과정을 거쳐야 해.
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex
테스트: "C:/Program Files/Epic Games/UE_5.8/Engine/Binaries/Win64/UnrealEditor.exe" "C:/Projects/Unreal/Asteria/Asteria.uproject" /Game/Map/Main -ExecCmds="Automation RunTests Asteria.Npc.Loop" -TestExit="Automation Test Queue Empty" -unattended -nosplash -nopause -log -abslog="{LOG_DIR}/test.log" -ReportExportPath="{LOG_DIR}/report"

[1단계]
Asteria.Build.cs (수정, 조건부)
PrivateDependencyModuleNames: 에디터 빌드에서만 CQTest, UnrealEd 추가 (수정, 조건부)


[2단계]
Tests/Npc/NpcLoopNetworkTest.cpp (신규)
구성: Listen Server, 클라 0, GameMode /Game/Blueprints/BP_GameMode.BP_GameMode_C, 테스트 경로 Asteria.Npc.Loop
NPC 퀘스트 루프 한 바퀴: [서버] 세션 시작, 맵 스포너가 만든 AAsteriaNpc 1명의 NpcId·NpcRnk 기록 → [서버] NPC가 PointType=QuestBoard 접근 지점과 2D 거리 200 이내에 도달(제한 60초) → [서버] QuestAssignments에 Party에 NpcId를 가진 State=Assigned Assignment 생김, 그 QuestId의 QuestPull 항목 QuestRnk ≤ NpcRnk(제한 10초) → [서버] 같은 AssignmentId가 State=Submitted, 그 시점에 NPC가 PointType=Counter 접근 지점과 2D 거리 200 이내(제한 60초) → [서버] 호스트 소유 폰(AAsteriaPlayer)의 Server_AcceptQuestAssignment(AssignmentId) 호출 → [서버] 같은 AssignmentId가 State=Accepted → [서버] NPC가 PointType=Dungeon 접근 지점과 2D 거리 200 이내에 도달, 그 시점에 State=Accepted(제한 60초) → [서버] 같은 AssignmentId가 State=SubmitForSettled, 그 시점에 NPC가 PointType=Counter 접근 지점과 2D 거리 200 이내(제한 60초) → [서버] 호스트 소유 폰의 Server_SettleQuestAssignment(AssignmentId) 호출 → [서버] 같은 AssignmentId가 State=Settled로 QuestAssignments에 남음 → [서버] NPC가 PointType=QuestBoard 접근 지점과 2D 거리 200 이내에 도달(제한 60초) → [서버] 이전과 다른 AssignmentId로 Party에 NpcId를 가진 State=Assigned Assignment 생김, 이전 AssignmentId는 State=Settled 유지(제한 10초) (정상, 코드 기준)
없는 ID 컨펌: [서버] NPC의 Assignment가 State=Submitted(제한 60초) → [서버] 호스트 소유 폰의 Server_AcceptQuestAssignment를 QuestAssignments에 없는 AssignmentId로 호출 → [서버] NPC의 Assignment State=Submitted 유지, QuestAssignments 개수 불변 (예외, 추가, 코드 기준)
순서 어긋난 정산: [서버] NPC의 Assignment가 State=Submitted(제한 60초) → [서버] 호스트 소유 폰의 Server_SettleQuestAssignment(그 AssignmentId) 호출 → [서버] 같은 AssignmentId가 State=Submitted 유지 (예외, 추가, 코드 기준)
