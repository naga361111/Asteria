요청: 이제 Settle도 동일하게 작용해야 해. 제출 노드를 나누고, 대기 노드로 목적지까지 이동하고, Receivce 기능을 넣고. (수정: 플레이어 정산 컨펌은 신호일 뿐, Settled 전이·지급은 NPC 수령(Receive) 단계에서)
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/Common/Quest.h (수정)
EQuestAssignmentState::SettleConfirmed: SubmitForSettled와 Settled 사이에 추가. 플레이어가 정산 컨펌 신호를 보냈고 NPC의 창구 수령을 기다림 — 다음 행위자는 NPC(수령). 컨펌 신호의 기록이라 보드 표시·중복 컨펌 차단·신호 유실 방지의 근거 (신규, 추가)
EQuestAssignmentState::Settled 주석: NPC가 창구에서 수령해 보상·명성까지 정산됨 — terminal, 전이 직후 배열에서 제거 (수정)
EQuestAssignmentState 위 설명 주석: NPC 수령 시 Settled가 되고 Assignment와 퀘스트가 제거된다로 갱신 (수정)

Source/Asteria/GameState/Components/QuestService.h / .cpp (수정)
SettleQuestAssignment(): 플레이어 정산 컨펌 신호. SubmitForSettled→SettleConfirmed 전이와 OnQuestAssignmentSettled 통지만, 제거·지급 없음. 이름은 플레이어 입력 경로(Server_SettleQuestAssignment)와 맞춰 유지 - TransitionQuestAssignment() 호출 (수정)
ReceiveSettleQuestAssignment(): NPC가 창구에서 수령. SettleConfirmed→Settled 전이 후 Assignment와 그 퀘스트를 배열에서 제거하고 변경 통지, 실패(권위 없음·미존재·SettleConfirmed 아님) 시 false 및 아무것도 지우지 않음 - TransitionQuestAssignment() 호출, UCounterService::ReceiveSettleQuestAssignment()가 호출 (신규)
OnQuestAssignmentSettled 선언 주석: SubmitForSettled→SettleConfirmed(플레이어 컨펌) 신호, WaitForSettleConfirm의 대기 종료용이며 지급은 수령 시 CounterService가 한다로 갱신 (수정)

Source/Asteria/UI/Counter/WaitForSettle/SettleEntryObject.h (수정)
State: 이 칸이 그리는 Assignment의 상태(SubmitForSettled 또는 SettleConfirmed) — 칸 위젯이 버튼 활성·배경색을 가름 (신규)
클래스 주석: SubmitForSettled 또는 SettleConfirmed 상태인 Assignment의 view-model로 갱신 (수정)

Source/Asteria/BTNode/BTTask_SubmitSettle.h / .cpp (신규)
UBTTask_SubmitSettle(): 노드 이름 "Submit Settle". 실행별 상태가 없어 공유 인스턴스 그대로 (신규)
ExecuteTask(): 내 NpcId의 Cleared Assignment를 창구의 보상 대기 제출함에 올리고(Cleared→SubmitForSettled) 즉시 Succeeded, 대상이 없거나 제출이 거절되면 Failed - UQuestService::FindQuestAssignmentByNpc(), UCounterService::SubmitForSettleQuestAssignment() 호출 (신규)
NPC·GameState·서비스 확인: 폰이 AAsteriaNpc가 아니거나 QuestService·CounterService가 없으면 Failed (신규, 추가)


[2단계]
Source/Asteria/GameState/Components/CounterService.h / .cpp (수정)
GetGuildService(): 오너 GameState에서 GuildService를 찾음 — GetQuestService()와 같은 방식 (신규, 추가)
SettleQuestAssignment(): 코드 변경 없음, 주석만 플레이어 정산 컨펌 신호(SubmitForSettled→SettleConfirmed)로 갱신 — 지급하지 않음 (수정)
ReceiveSettleQuestAssignment(): NPC가 창구에서 수령. 수수료(RewardAmount*CommissionRate 반올림)를 길드 자금에, 퀘스트 등급의 명성을 길드 명성에 적립 — 지급은 수령 순간 서버에서 - UQuestService::ReceiveSettleQuestAssignment(), UGuildService::AddGuildFunds(), UGuildService::AddGuildReputation() 호출, BTTask_ReceiveSettle::ExecuteTask()가 호출 (신규)
지급 근거 확인: 수령 전에 SettleConfirmed Assignment·퀘스트 정의·명성 표 항목을 모두 찾고 지급액을 계산해 둠, 하나라도 없으면 수령하지 않고 false — 수령 후엔 근거가 제거되므로 (신규, 추가)
지급 순서: 수령이 성공한 뒤에만 지급 — 거절된 수령에 지급하거나 중복 지급하지 않음 (신규, 추가)

Source/Asteria/BTNode/BTTask_WaitForSettleConfirm.h / .cpp (수정)
전체 구조: BTTask_WaitForConfirmQuest와 같은 모양으로 재작성 — 노드 인스턴스화, 이동·회전 자체 구현, NodeMemory 구조체와 GetInstanceMemorySize()·InitializeMemory()·CleanupMemory() 삭제, 제출·지급 코드 삭제 (수정)
TargetType·AcceptableRadius·InterpSpeed·AngleTolerance: 에디터 설정 항목, TargetType 기본값 Lounge (신규)
TargetPoint·CachedOwnerComp·MoveRequestID·bFacing·QuestService·SettledHandle·WaitingAssignmentId: 실행 상태 멤버 (신규)
FindNearestSettleWaitPoint(): cpp 익명 네임스페이스의 가장 가까운 지점 탐색 — WaitForConfirmQuest.cpp의 FindNearestWaitPoint와 같은 동작, unity 빌드 이름 충돌을 피하려 다른 이름 - ExecuteTask()가 호출 (신규)
ExecuteTask(): 내 NpcId의 SubmitForSettled Assignment를 찾아 정산 컨펌 신호를 구독 → 가장 가까운 TargetType 지점으로 이동 요청, InProgress 반환 - FindNearestSettleWaitPoint(), UQuestService::FindQuestAssignmentByNpc(), OnQuestAssignmentSettled 구독, AAIController::MoveToActor() 호출 (수정)
이미 컨펌됨 확인: SubmitForSettled가 없고 SettleConfirmed가 있으면 제출과 대기 사이에 이미 컨펌된 것 → 이동 없이 즉시 Succeeded, 둘 다 없으면 Failed (신규, 추가)
지점 없음·이동 실패: 실패로 끝내지 않고 제자리에서 정산 컨펌 대기, 경고 로그 (신규, 추가)
정산 컨펌 신호 람다: 내 WaitingAssignmentId면 이동 중이든 도착 후든 즉시 Succeeded (수정)
OnMoveCompleted(): 스테일 요청 무시, 도착 성공이면 회전 페이즈, 실패면 경고 로그 후 제자리 대기 — 태스크를 끝내지 않음 - ReceiveMoveCompleted에 바인딩 (신규)
TickTask(): 회전 페이즈에서 지점 yaw로 보간 회전, 다 돌면 회전만 종료 (신규)
OnTaskFinished(): 구독 해제, 이 태스크의 이동이 진행 중이면 정지, 이동 완료 콜백 해제, 실행 상태 초기화 - UnbindMoveCompleted() 호출 (수정)
UnbindMoveCompleted(): 이동 완료 콜백 해제와 MoveRequestID 초기화 (신규)
StepFacing(): 지점 yaw로 한 틱 회전, 허용 각도 이내면 정렬 후 true (신규)

Source/Asteria/UI/Counter/WaitForSettle/SettleBoardWidget.cpp (수정)
RefreshSettlements(): 표시 대상을 SubmitForSettled만 → SubmitForSettled 또는 SettleConfirmed로 확장, 각 칸 Entry에 State 채움. 수령으로 제거되면 보드에서 사라짐, 필터 주석 갱신 - USettleEntryObject::State 설정 (수정)

Source/Asteria/UI/Counter/WaitForSettle/SettleTileEntryWidget.h / .cpp (수정)
StateBorder: 칸 배경 Border. WBP_CounterForSettleTileEntry에 같은 이름의 Border가 있어야 함 - BindWidget (신규)
WaitingReceiveColor: 수령 대기(SettleConfirmed) 배경색, WBP Class Defaults에서 조정 — CounterTileEntryWidget과 같은 기본값 (신규)
DefaultBorderColor: WBP에 설정된 원래 배경색을 기억하는 멤버 (신규)
State: 이 칸이 그리는 Assignment 상태 멤버 (신규)
NativeOnInitialized(): 기존 버튼 구독에 더해 StateBorder의 원래 색을 DefaultBorderColor로 기억 - UBorder::GetBrushColor() 호출 (수정)
NativeOnListItemObjectSet(): 기존 표시에 더해 State 저장, SubmitForSettled면 버튼 활성·원래 색, SettleConfirmed면 버튼 비활성·WaitingReceiveColor — 재사용 위젯이라 매번 칠함 - ButtonText->SetIsEnabled(), UBorder::SetBrushColor() 호출 (수정)
HandleSettleClicked(): State가 SubmitForSettled가 아니면 서버 요청을 보내지 않음 — 수령 대기 칸의 중복 컨펌 차단(서버도 상태 검사로 거절) (수정, 추가)
클래스 주석: WBP가 제공하는 위젯 목록에 StateBorder 추가 (수정)


[3단계]
Source/Asteria/BTNode/BTTask_ReceiveSettle.h / .cpp (신규)
UBTTask_ReceiveSettle(): 노드 이름 "Receive Settle". 실행별 상태가 없어 공유 인스턴스 그대로 (신규)
ExecuteTask(): 내 NpcId의 SettleConfirmed Assignment를 창구에서 수령(Settled 전이·지급·제거)하고 즉시 Succeeded, 대상이 없거나 수령이 거절되면 Failed - UQuestService::FindQuestAssignmentByNpc(), UCounterService::ReceiveSettleQuestAssignment() 호출 (신규)
NPC·GameState·서비스 확인: 폰이 AAsteriaNpc가 아니거나 QuestService·CounterService가 없으면 Failed (신규, 추가)


[4단계]
Content/Blueprints/Behavior/BT_Npc.uasset (수정, 에디터 작업)
정산 구간 순서: Do Quest → MoveToApproachPoint(Counter) → Submit Settle → Wait For Settle Confirm(Lounge) → MoveToApproachPoint(Counter) → Receive Settle → MoveToApproachPoint(Disappear) → Despawn — Submit Settle이 없으면 대기 노드가 Failed (수정)

Content/UI/Counter/WBP_CounterForSettleTileEntry.uasset (수정, 에디터 작업)
Border 이름 변경: 칸 배경 Border 이름을 StateBorder로 — 이름이 다르면 WBP 컴파일 에러 (수정)
