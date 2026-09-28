요청: 상세 설계 진행. (플레이어가 CounterBoard에서 컨펌해도 Assignment는 보드에서 바로 사라지지 않고, NPC가 카운터로 와서 회수해야 사라진다. Accepted 뒤에 Received 상태를 추가하는 설계)
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/Common/Quest.h (수정)
EQuestAssignmentState::Received: Accepted와 Cleared 사이에 추가. NPC가 창구에서 컨펌된 퀘스트를 회수함 — 다음 행위자는 NPC(던전 수행) (신규)
EQuestAssignmentState::Accepted 주석: 플레이어가 컨펌했고 NPC의 창구 회수를 기다림 — 다음 행위자는 NPC(회수)로 갱신 (수정)


[2단계]
Source/Asteria/GameState/Components/QuestService.h / .cpp (수정)
ReceiveQuestAssignment(): Accepted→Received 전이, 실패(권위 없음·미존재·상태 불일치) 시 false - TransitionQuestAssignment() 호출, UCounterService::ReceiveQuestAssignment()가 호출 (신규)
ClearQuestAssignment(): 시작 상태를 Accepted → Received로 변경, 주석 갱신 - TransitionQuestAssignment() 호출 (수정)

Source/Asteria/UI/Counter/WaitForAccept/CounterEntryObject.h (수정)
State: 이 칸이 그리는 Assignment의 상태(Submitted 또는 Accepted) — 칸 위젯이 버튼 활성 여부를 가름 (신규)

Source/Asteria/BTNode/BTTask_DoQuest.cpp (수정)
ExecuteTask(): 수행할 Assignment 탐색 상태를 Accepted → Received로 변경, 주석 갱신 - UQuestService::FindQuestAssignmentByNpc() 호출 (수정)


[3단계]
Source/Asteria/GameState/Components/CounterService.h / .cpp (수정)
ReceiveQuestAssignment(): NPC가 창구에서 컨펌된 퀘스트를 회수(Accepted→Received). 검증은 하지 않고 위임만 — SubmitQuestAssignment와 같은 창구 경계 - UQuestService::ReceiveQuestAssignment() 호출, BTTask_ReceiveQuest::ExecuteTask()가 호출 (신규)

Source/Asteria/UI/Counter/WaitForAccept/CounterBoardWidget.h / .cpp (수정)
RefreshSubmissions(): 표시 대상을 Submitted만 → Submitted 또는 Accepted로 확장, 각 칸 Entry에 State 채움. Received가 되면 필터에서 빠져 보드에서 사라짐 - UCounterEntryObject::State 설정 (수정)
클래스·필터 주석: 제출함 = Submitted(컨펌 대기) + Accepted(회수 대기) 뷰로 갱신 (수정)

Source/Asteria/UI/Counter/WaitForAccept/CounterTileEntryWidget.h / .cpp (수정)
State: 이 칸이 그리는 Assignment 상태 멤버 — 재사용 위젯이라 바인딩마다 갱신 (신규)
NativeOnListItemObjectSet(): 기존 표시에 더해 Entry의 State를 저장하고, Submitted면 컨펌 버튼 활성, Accepted면 비활성 - ButtonText->SetIsEnabled() 호출 (수정)
HandleAcceptClicked(): State가 Submitted가 아니면 서버 요청을 보내지 않음 — 회수 대기 칸의 중복 컨펌 차단(서버도 상태 검사로 거절) (수정, 추가)


[4단계]
Source/Asteria/BTNode/BTTask_ReceiveQuest.h / .cpp (신규)
UBTTask_ReceiveQuest(): 노드 이름 "Receive Quest". 실행별 상태가 없어 공유 인스턴스 그대로 (신규)
ExecuteTask(): 내 NpcId의 Accepted Assignment를 창구에서 회수(Accepted→Received)하고 즉시 Succeeded, 대상이 없거나 회수가 거절되면 Failed - UQuestService::FindQuestAssignmentByNpc(), UCounterService::ReceiveQuestAssignment() 호출 (신규)
NPC·GameState·서비스 확인: 폰이 AAsteriaNpc가 아니거나 QuestService·CounterService가 없으면 Failed (신규, 추가)


[5단계]
Content/Blueprints/Behavior/BT_Npc.uasset (수정, 에디터 작업)
수주 구간 순서: Submit Quest → Wait For Confirm Quest(Lounge) → MoveToApproachPoint(Counter) → Receive Quest → MoveToApproachPoint(Dungeon) → Do Quest — Receive Quest가 없으면 Do Quest가 Received를 못 찾아 Failed (수정)
