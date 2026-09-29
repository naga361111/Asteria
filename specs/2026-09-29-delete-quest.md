요청: DeleteQuest 노드를 만들었어. 여기에 Assignment를 해제하고 무조건 퀘스트를 지우는 로직을 넣어줘.
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/GameState/Components/QuestService.h / .cpp (수정)
DeleteQuestAssignment(): Assignment를 QuestAssignments에서, 그 퀘스트를 QuestPull에서 남은 시간과 상태에 관계없이 제거. 성공 시 true - BTTask_DeleteQuest::ExecuteTask(), ReceiveSettleQuestAssignment()가 호출 (신규)
권위 확인: 호스트가 아니면 아무것도 지우지 않고 false (신규, 추가)
실재 검증: Assignment가 없으면 false. 제거 전에 QuestId를 복사해 둠 — 제거 후 포인터 무효 (신규, 추가)
변경 통지: 서버는 OnRep이 자동 호출되지 않으므로 OnQuestAssignmentsChanged·OnQuestPullChanged를 Broadcast — 퀘스트 보드·수주 제출함·정산 제출함 위젯이 기존 구독으로 갱신됨 (신규, 추가)
ReceiveSettleQuestAssignment(): Settled 전이 후 Assignment·퀘스트를 직접 지우던 부분을 DeleteQuestAssignment() 호출로 대체 — 같은 제거 로직을 두 벌 두지 않음, 동작은 동일 (수정, 추가)


[2단계]
Source/Asteria/BTNode/BTTask_DeleteQuest.h / .cpp (수정)
UBTTask_DeleteQuest(): NodeName "Delete Quest" 설정, 실행별 상태가 없으므로 노드 인스턴스화 안 함 (신규)
ExecuteTask(): 내 NpcId가 든 Assignment를 모든 진행 상태(Assigned~SettleConfirmed)에서 찾아 전부 삭제하고 즉시 끝남 — 떠나는 NPC가 든 것이 하나도 남지 않게, 서버 권위 쓰기는 QuestService가 함 - UQuestService::FindQuestAssignmentByNpc(), UQuestService::DeleteQuestAssignment() 호출 (신규)
삭제할 Assignment 없음: 퀘스트 없이 떠나는 경우라 Succeeded — 실패로 끝나면 뒤따르는 떠나기(Exit 이동·Despawn)가 막힘 (신규, 추가)
삭제 실패: DeleteQuestAssignment가 false면 경고 로그 후 Failed (신규, 추가)
NPC·GameState·QuestService 없음: Failed (신규, 추가)
