요청: WaitForSettle 에서 실패한 퀘스트도 제출 가능하도록. 다만 이후 Settle 정산 노드에서 실패한 퀘스트는 보상(명성과 돈)을 주지 않도록 설계. 실패한 퀘스트는 지워지지 않고, 만료 시간이 게임 시간 기준 5분 이상 남았으면 다시 Available 상태가 되어야 해. 물론 Assignment는 지워지고.
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/Common/Quest.h (수정)
FQuestAssignment::bQuestFailed: 수행이 실패로 끝났는지 기록하는 멤버 변수. State는 Failed 뒤 SubmitForSettled·SettleConfirmed로 계속 넘어가 실패 사실이 사라지므로 정산 시점까지 남길 근거로 따로 둠. QuestAssignments와 함께 복제됨 - UQuestService::FailQuestAssignment()가 씀, UQuestService::ReceiveSettleQuestAssignment()·UCounterService::ReceiveSettleQuestAssignment()가 읽음 (신규, 추가)


[2단계]
Source/Asteria/GameState/Components/QuestService.h / .cpp (수정)
FailQuestAssignment(): Received→Failed 전이가 받아들여지면 그 Assignment의 bQuestFailed를 설정. 설정도 같은 변경 통지에 실림 - TransitionQuestAssignment() 호출 (수정)
SubmitForSettleQuestAssignment(): Cleared뿐 아니라 Failed에서도 SubmitForSettled로 전이. 권위·실재·상태 검사와 변경 통지는 공통 경로가 함 - TransitionQuestAssignment() 호출, UCounterService::SubmitForSettleQuestAssignment()가 호출 (수정)
ReleaseQuestAssignment(): Assignment를 QuestAssignments에서 제거하고, 퀘스트는 만료까지 MinReturnRemainingMinutes(5 게임 분) 미만일 때만 QuestPull에서 제거. 퀘스트가 남으면 살아있는 Assignment가 없으므로 다시 집을 수 있는 상태가 됨. 변경 통지 포함. 상태 검사는 하지 않는 private 함수 — 기존 ReturnQuestAssignment() 본문의 제거 부분을 옮긴 것 - ReturnQuestAssignment()·ReceiveSettleQuestAssignment()가 호출 (신규, 추가)
ReleaseQuestAssignment() 입력 검증: GameClockService가 없어 남은 시간을 판정할 수 없거나 Assignment가 없으면 아무것도 지우지 않고 false (신규, 추가)
ReturnQuestAssignment(): 권위·수주 단계(Assigned·Submitted·Accepted) 검사는 그대로 두고 제거는 ReleaseQuestAssignment()에 맡김. 동작은 동일 - ReleaseQuestAssignment() 호출 (수정)
ReceiveSettleQuestAssignment(): SettleConfirmed→Settled 전이 후 bQuestFailed면 ReleaseQuestAssignment(), 아니면 기존대로 DeleteQuestAssignment() - TransitionQuestAssignment(), ReleaseQuestAssignment(), DeleteQuestAssignment() 호출, UCounterService::ReceiveSettleQuestAssignment()가 호출 (수정)


[3단계]
Source/Asteria/BTNode/BTTask_SubmitSettle.cpp (수정)
ExecuteTask(): 올릴 Assignment를 Cleared에서 찾고, 없으면 Failed에서 찾음. 이후 제출은 기존대로 - UQuestService::FindQuestAssignmentByNpc(), UCounterService::SubmitForSettleQuestAssignment() 호출 (수정)

Source/Asteria/GameState/Components/CounterService.h / .cpp (수정)
SubmitForSettleQuestAssignment(): 변경 없음, 주석의 전이 설명만 Cleared 또는 Failed→SubmitForSettled로 고침 (재사용: Source/Asteria/GameState/Components/CounterService.cpp)
ReceiveSettleQuestAssignment(): Assignment의 bQuestFailed를 수령 전에 값으로 복사해 둠. 실패한 퀘스트면 수령만 하고 길드 자금·명성은 적립하지 않음. 실패한 퀘스트는 명성 표 조회가 필요 없으므로 표·등급 항목이 없어도 수령을 막지 않음. 성공한 퀘스트는 기존대로 - UQuestService::ReceiveSettleQuestAssignment(), UGuildService::AddGuildFunds(), UGuildService::AddGuildReputation() 호출 (수정)
