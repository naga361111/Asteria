요청: GuildService에 등급 업에 필요한 명성을 저장해두는 변수를 만들고, 해당 명성을 받은 인자만큼 증가시키는 헬퍼 함수를 구현. 이후 퀘스트 정산하는 Settle BT에서 해당 함수를 호출하도록 설정. 명성을 증가시키는 헬퍼 함수에는 일정 명성을 넘으면 등급업 되는 기능이 있어야 해.
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/GameState/Components/GuildService.h / .cpp (수정)
GuildReputation: 길드 누적 명성, 복제되며 쓰기는 서버 권위. 승급해도 초기화하지 않음 (신규)
ReputationData: 명성 표 데이터 에셋 참조, BP_GameState의 컴포넌트 기본값에서 지정(EditDefaultsOnly) - UGuildReputationData 참조, 재사용: Source/Asteria/Data/GuildReputationData.h (신규, 추가)
GetLifetimeReplicatedProps(): GuildReputation 복제 등록 추가 (수정)
AddGuildReputation(): 누적 명성을 Amount만큼 늘리고, 다음 등급의 도달 기준을 넘으면 등급 업. 한 번에 여러 기준을 넘으면 연속 승급, S급에서 멈춤 - ReputationData의 ReputationToReachRank 참조, BTTask_WaitForSettleConfirm이 호출 (신규)
AddGuildReputation 검증: 오너에 권위가 없거나 0 이하 명성이면 false (신규, 추가)
AddGuildReputation 데이터 검증: ReputationData가 없거나 다음 등급 기준이 표에 없으면 명성만 쌓고 승급 판정을 멈추며 경고 로그 (신규, 추가)


[2단계]
Source/Asteria/BTNode/BTTask_WaitForSettleConfirm.cpp (수정)
FBTWaitForSettleConfirmMemory::Reputation: 이번 정산에서 길드가 얻을 명성 (신규)
ExecuteTask(): 퀘스트 등급으로 GuildService의 ReputationData에서 얻을 명성을 조회해 메모리에 보관 - UGuildService::ReputationData, UGuildReputationData::ReputationByQuestRank 참조 (수정)
ExecuteTask 명성 표 검증: ReputationData가 없거나 표에 퀘스트 등급이 없으면 Failed (신규, 추가)
정산 람다: 수수료 입금과 함께 명성 적립 - UGuildService::AddGuildReputation() 호출 (수정)
OnTaskFinished(): Reputation 초기화 (수정)
