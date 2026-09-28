요청: 해당 BT에 Npc가 제출 후, Lounge 목적지로 이동하도록 해줘. 여기서는 BT MoveToApproach 를 그대로 사용. (수정: 제출 노드와 대기 노드로 분리. 대기 노드는 MoveToApproach 동작을 그대로 자체 구현하면서, 이동 중 컨펌 시 즉시 Succeeded, 도착 후에는 컨펌까지 대기. MoveToApproachPoint는 수정하지 않음)
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/BTNode/BTTask_MoveToApproachPoint.h / .cpp (수정)
전체: 이전 구현의 변경(FinishMove·OnTaskFinished·protected 이동·bNotifyTaskFinished)을 모두 되돌려 HEAD 상태로 복원 — 이 태스크는 유지 대상 (수정)

Source/Asteria/BTNode/BTTask_SubmitQuest.h / .cpp (재사용: Source/Asteria/BTNode/BTTask_SubmitQuest.cpp)
전체: 이전 구현 그대로 유지 — 내 NpcId의 Assigned Assignment를 창구에 제출하고 즉시 Succeeded (재사용: Source/Asteria/BTNode/BTTask_SubmitQuest.cpp)

Source/Asteria/BTNode/BTTask_WaitForConfirmQuest.h / .cpp (수정)
부모 클래스: UBTTask_MoveToApproachPoint → UBTTaskNode로 복귀. 이동 완료 콜백·실행 상태를 멤버로 들기 위해 bCreateNodeInstance, 회전용 bNotifyTick, 정리용 bNotifyTaskFinished 켬 (수정)
TargetType·AcceptableRadius·InterpSpeed·AngleTolerance: MoveToApproachPoint와 같은 에디터 설정 항목, TargetType 기본값 Lounge (신규)
TargetPoint·CachedOwnerComp·MoveRequestID·bFacing: MoveToApproachPoint와 같은 이동·회전 실행 상태 멤버 (신규)
QuestService·AcceptedHandle·WaitingAssignmentId: 컨펌 대기 실행 상태 멤버 (수정)
FindNearestWaitPoint(): cpp 익명 네임스페이스에 MoveToApproachPoint.cpp와 같은 탐색 함수 — MoveToApproachPoint를 건드리지 않기 위해 복제, unity 빌드 이름 충돌을 피하려 다른 이름 - ExecuteTask()가 호출 (신규)
ExecuteTask(): 내 NpcId의 Submitted Assignment를 찾아 컨펌 알림 구독 → 가장 가까운 TargetType 지점으로 이동 요청(이미 도착이면 회전 페이즈), InProgress 반환 - FindNearestWaitPoint(), UQuestService::FindQuestAssignmentByNpc(), OnQuestAssignmentAccepted 구독, AAIController::MoveToActor() 호출 (수정)
이미 컨펌됨 확인: Submitted가 없고 Accepted가 있으면 제출과 대기 사이에 이미 컨펌된 것 → 이동 없이 즉시 Succeeded, 둘 다 없으면 Failed (신규, 추가)
지점 없음·이동 요청 실패: 실패로 끝내지 않고 제자리에서 컨펌 대기 계속, 경고 로그 — 대기 자리는 연출이라 수주 흐름 유지 (신규, 추가)
컨펌 알림 람다: 내 WaitingAssignmentId면 이동 중이든 도착 후든 즉시 Succeeded (수정)
OnMoveCompleted(): 스테일 요청 무시, 도착 성공이면 회전 페이즈 진입(즉시 회전이면 바로 정렬), 실패면 경고 로그 후 제자리 대기 — 어느 경우에도 태스크를 끝내지 않음 - ReceiveMoveCompleted에 바인딩 (신규)
TickTask(): 회전 페이즈에서 지점 yaw로 보간 회전, 다 돌면 회전 페이즈 종료 후 컨펌 대기 계속 — 태스크를 끝내지 않음 (신규)
OnTaskFinished(): 컨펌 구독 해제, 이 태스크의 이동 요청이 진행 중이면 이동 정지, 이동 완료 콜백 해제, 실행 상태 초기화 — 성공·실패·중단 모든 경로 공통 - UnbindMoveCompleted() 호출 (수정)
UnbindMoveCompleted(): 이동 완료 콜백 해제와 MoveRequestID 초기화 (신규)
StepFacing(): 지점 yaw로 한 틱 회전, 허용 각도 이내면 정렬 후 true (신규)


[2단계]
Content/Blueprints/Behavior/BT_Npc.uasset (수정, 에디터 작업)
수주 구간 순서: MoveToApproachPoint(Counter) → Submit Quest → Wait For Confirm Quest(TargetType=Lounge) — 대기 노드는 더 이상 제출하지 않으므로 Submit Quest가 없으면 대기 노드가 Failed (수정)
Lounge 지점 배치: 맵에 PointType=Lounge인 ApproachPointActor 최소 1개 — 없으면 창구에서 제자리 대기 (추가)
