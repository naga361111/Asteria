요청: 정산이 끝난 후, 완료된 Assignment와 퀘스트를 배열에서 완전히 지우는 기능을 추가.
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/GameState/Components/QuestService.h / .cpp (수정)
SettleQuestAssignment(): SubmitForSettled→Settled 전이와 정산 알림 뒤에, 그 Assignment를 QuestAssignments에서, 해당 QuestId의 퀘스트를 QuestPull에서 제거 - TransitionQuestAssignment() 호출(재사용: Source/Asteria/GameState/Components/QuestService.cpp), OnQuestAssignmentSettled.Broadcast() 호출, CounterService::SettleQuestAssignment()가 호출 (수정)
SettleQuestAssignment 순서 보장: 정산 알림(BT 람다의 수수료·명성 지급)이 제거보다 먼저 끝나야 함 (수정, 추가)
SettleQuestAssignment 제거 조건: 전이가 성공했을 때만 제거. 권위 없음·미존재·상태 불일치로 전이가 실패하면 아무것도 지우지 않음 (수정, 추가)
SettleQuestAssignment 변경 통지: 제거 후 서버에서 OnQuestAssignmentsChanged, OnQuestPullChanged를 직접 Broadcast. 클라는 기존 OnRep_QuestAssignments/OnRep_QuestPull로 통지됨 - QuestBoardWidget, SettleBoardWidget, CounterBoardWidget이 구독 (수정, 추가)
SettleQuestAssignment 주석: "유일한 terminal 전이" 설명에 정산 직후 Assignment·퀘스트가 배열에서 제거된다는 내용 반영 (수정, 추가)

Source/Asteria/Common/Quest.h (수정)
EQuestAssignmentState 주석: "완수 이후(Cleared·Settled)는 값으로 남긴다"를 정산 확정 후 배열에서 제거된다는 내용으로 수정 (수정, 추가)
EQuestAssignmentState::Settled 주석: 정산 확정 직후 제거되므로 배열에 남아 있지 않는다는 내용 반영 (수정, 추가)

Source/Asteria/UI/Quest/QuestBoardWidget.cpp (수정)
NativeConstruct() 주석: "QuestPull은 정적" 설명을 정산 시 QuestPull에서 퀘스트가 제거된다는 내용으로 수정 (수정, 추가)
