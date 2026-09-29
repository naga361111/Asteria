요청: DoQuest에 퀘스트 실패 기능을 구현하고 현 데이터를 적용
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/Common/Quest.h (수정)
EQuestAssignmentState::Failed: 수행이 실패로 끝났음을 나타내는 상태. Cleared 바로 다음에 둠 — BTTask_DeleteQuest가 Assigned~SettleConfirmed 숫자 범위로 훑으므로 그 안에 있어야 떠나는 NPC의 실패 Assignment도 지워짐. 다른 코드는 값을 숫자로 저장하지 않아 뒤 값이 밀려도 영향 없음 - UQuestService::FailQuestAssignment()가 씀 (신규)


[2단계]
Source/Asteria/GameState/Components/QuestService.h / .cpp (수정)
BaseFailChance: 동급 퀘스트의 실패 확률 0.10. 익명 네임스페이스 상수 - QuestFailChance()가 읽음 (신규)
UpperFailChance: 한 단계 위 퀘스트에서 BaseFailChance에 더하는 실패 확률 0.30. 익명 네임스페이스 상수 - QuestFailChance()가 읽음 (신규)
LowerFailFalloff: 한 단계 아래로 내려갈 때마다 실패 확률에 곱하는 비율 0.5. 익명 네임스페이스 상수 - QuestFailChance()가 읽음 (신규)
QuestFailChance(): Npc 등급과 퀘스트 등급의 차이로 실패 확률을 계산. 동급·아래는 BaseFailChance × LowerFailFalloff^(내려간 단계 수), 위는 BaseFailChance + UpperFailChance. RollQuestRank() 옆 익명 네임스페이스 함수 - RollQuestFailure()가 호출 (신규)
RollQuestFailure(): Assignment의 퀘스트 등급과 인자로 받은 Npc 등급으로 실패 여부를 한 번 굴려 실패면 true. 내부 랜덤이라 서버에서만 의미 있음 - FindQuestAssignment(), QuestFailChance() 호출, BTTask_DoQuest::ExecuteTask()가 호출 (신규)
RollQuestFailure() 입력 검증: Assignment나 그 퀘스트가 QuestPull에 없으면 false — 실패 판정 대신 뒤이은 전이가 거절하도록 둠 (신규, 추가)
FailQuestAssignment(): Received→Failed 전이. ClearQuestAssignment()와 같은 자리의 전이 함수, 권위·실재·현재 상태 검사와 변경 통지는 공통 경로가 함 - TransitionQuestAssignment() 호출, BTTask_DoQuest::TickTask()가 호출 (신규)


[3단계]
Source/Asteria/BTNode/BTTask_DoQuest.h / .cpp (수정)
FBTDoQuestMemory::bFailed: 이번 수행이 실패로 판정됐는지 기록하는 멤버 변수 (신규)
ExecuteTask(): Assignment를 찾은 뒤 Npc->NpcRnk로 실패 여부를 굴려 bFailed에 기록하고 시작 로그에 함께 남김 - UQuestService::RollQuestFailure() 호출 (수정)
TickTask(): QuestDuration이 지나면 bFailed면 실패 전이, 아니면 완료 전이. 어느 쪽이든 전이가 받아들여지면 Succeeded로 끝나 다음 단계로 넘어감. 전이가 거절되면(Assignment 소멸·상태 불일치) 기존대로 Failed. 로그에 성공/실패 결과 표시 - UQuestService::FailQuestAssignment(), UQuestService::ClearQuestAssignment() 호출 (수정)
InitializeMemory(), GetInstanceMemorySize(): 변경 없음 (재사용: Source/Asteria/BTNode/BTTask_DoQuest.cpp)
