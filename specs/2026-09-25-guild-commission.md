요청: Npc가 카운터에서 정산 받는 BT 단계에서, 자금 거래가 이루어져야 해. 퀘스트 자체에 보상 액수와 수수료 비율이 설정되고 (보상 액수는 등급별로 상이. 그리고 각 등급별 액수 범위가 존재. 이거 너가 임의로 결정). 당연히 퀘스트 초기화 시 얘들도 같이 설정되어야 해. GameState 에 길드 공통 (당연히 복제) 지갑을 넣고 여기에 돈을 저장하는데, Npc 정산 시 보상 * 수수료 를 길드 지갑에 추가. Npc에 가는건 없음.
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/Common/Quest.h (수정)
RewardAmount: 퀘스트 보상 액수(정수). QuestPull과 함께 복제됨 - QuestService 생성자가 설정, BTTask_WaitForSettleConfirm이 읽음 (신규)
CommissionRate: 보상 중 길드가 가져가는 수수료 비율(0~1 실수) - QuestService 생성자가 설정, BTTask_WaitForSettleConfirm이 읽음 (신규)

Source/Asteria/GameState/AsteriaGameState.h / .cpp (수정)
GuildFunds: 길드 공통 지갑 잔액(정수). 복제됨, 쓰기는 서버만 - AddGuildFunds()가 씀 (신규)
GetLifetimeReplicatedProps(): GuildFunds를 복제 목록에 등록 (신규, 추가)
AddGuildFunds(): 인자로 받은 액수만큼 길드 잔액을 늘리는 헬퍼 - BTTask_WaitForSettleConfirm이 호출 (신규)
AddGuildFunds() 권한 확인: 서버(호스트)가 아니면 거절하고 false 반환 (신규, 추가)
AddGuildFunds() 입력 검증: 0 이하 금액은 거절하고 false 반환 (신규, 추가)


[2단계]
Source/Asteria/GameState/Components/QuestService.cpp (수정)
RewardRangeByRank: 등급별 보상 액수 범위 표. F 10~30, E 30~80, D 80~200, C 200~500, B 500~1200, A 1200~3000, S 3000~8000 - 생성자가 참조 (신규)
CommissionRate 범위: 퀘스트마다 0.1~0.3 사이 랜덤 - 생성자가 참조 (신규)
UQuestService(): 퀘스트 생성 시 등급을 정한 뒤, 그 등급 범위에서 RewardAmount를, 수수료 범위에서 CommissionRate를 랜덤으로 설정 (수정)

Source/Asteria/BTNode/BTTask_WaitForSettleConfirm.cpp (수정)
FBTWaitForSettleConfirmMemory::GameState: 정산 시 입금할 GameState 약한 참조 (신규, 추가)
FBTWaitForSettleConfirmMemory::Commission: 이번 정산에서 길드에 들어갈 금액(RewardAmount * CommissionRate 반올림 정수) (신규, 추가)
ExecuteTask(): 내 Cleared Assignment의 QuestId로 QuestPull에서 퀘스트를 찾아 Commission을 계산해 메모리에 저장하고 GameState를 기록. 이후 기존대로 구독 → 정산 대기 제출 (수정)
ExecuteTask() 검증: QuestId에 해당하는 퀘스트를 QuestPull에서 못 찾으면 Failed (신규, 추가)
정산 확정 람다: 내 AssignmentId가 Settled 되면 GameState의 AddGuildFunds(Commission)를 호출한 뒤 FinishLatentTask. NPC에게는 아무것도 주지 않음 - AAsteriaGameState::AddGuildFunds() 호출 (수정)
OnTaskFinished(): 기존 정리에 GameState·Commission 초기화 추가 (수정)
QuestService::SettleQuestAssignment(): SubmitForSettled→Settled 전이 후 OnQuestAssignmentSettled 브로드캐스트. 중복 정산은 From 검사가 막아 람다가 두 번 불리지 않음 (재사용: Source/Asteria/GameState/Components/QuestService.cpp)
