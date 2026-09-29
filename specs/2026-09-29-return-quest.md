요청: ReturnQuest 에 반환 기능을 넣어야 해. Assignment를 제거하고, 여기에 할당된 Quest가 만료 시간까지 5분 이상 남았으면 잡을 수 있는 상태로 되돌리고, 아니면 퀘스트 자체를 지워야 해.
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/GameState/Components/QuestService.h / .cpp (수정)
MinReturnRemainingMinutes: 반환 시 퀘스트를 보드에 되돌릴 최소 남은 시간(게임 분) 5 — cpp 익명 네임스페이스 상수, 기존 QuestClockIntervalSeconds 등과 같은 자리 (신규)
ReturnQuestAssignment(): Assignment를 QuestAssignments에서 제거해 반환 — 집힘 여부는 살아있는 Assignment 존재로 파생되므로 제거만으로 퀘스트가 다시 잡을 수 있는 상태가 됨. 성공 시 true - BTTask_ReturnQuest::ExecuteTask()가 호출 (신규)
권위 확인: 호스트가 아니면 아무것도 지우지 않고 false (신규, 추가)
실재·상태 검증: Assignment가 없거나 State가 수주 단계(Assigned·Submitted·Accepted)가 아니면 false — NPC가 회수한 뒤(Received 이후)의 퀘스트는 수행·정산 중이라 보드로 되돌리면 안 됨 (신규, 추가)
남은 시간 판정: 제거 전에 QuestId를 복사해 두고, 해당 퀘스트의 만료 게임 분 − 지금 게임 분이 MinReturnRemainingMinutes 미만이면 QuestPull에서도 제거, 이상이면 QuestPull에 남김 - UGameClockService::GetGameMinutes() 호출 (신규)
GameClockService 없음: 남은 시간을 판정할 수 없으므로 아무것도 지우지 않고 false (신규, 추가)
변경 통지: 서버는 OnRep이 자동 호출되지 않으므로 OnQuestAssignmentsChanged를 항상, 퀘스트를 지웠으면 OnQuestPullChanged도 Broadcast — 퀘스트 보드·창구 제출함 위젯이 기존 구독으로 갱신됨 (신규, 추가)


[2단계]
Source/Asteria/BTNode/BTTask_ReturnQuest.h / .cpp (수정)
UBTTask_ReturnQuest(): NodeName "Return Quest" 설정, 실행별 상태가 없으므로 노드 인스턴스화 안 함 (신규)
ExecuteTask(): 내 NpcId의 수주 단계 Assignment를 Submitted·Assigned·Accepted 순으로 찾아 반환하고 즉시 끝남 — 서버 권위 쓰기는 QuestService가 함 - UQuestService::FindQuestAssignmentByNpc(), UQuestService::ReturnQuestAssignment() 호출 (신규)
반환할 Assignment 없음: 퀘스트를 못 집고 대기하다 떠나는 경우라 Succeeded — 실패로 끝나면 뒤따르는 떠나기(Exit 이동·Despawn)가 막힘 (신규, 추가)
반환 실패: ReturnQuestAssignment가 false면 경고 로그 후 Failed (신규, 추가)
NPC·GameState·QuestService 없음: Failed (신규, 추가)
