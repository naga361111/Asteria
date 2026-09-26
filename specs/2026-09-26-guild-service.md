요청: GameState 에 새 컴포넌트가 필요해. 길드의 상태를 관리하는 컴포넌트야. 길드의 자금도 여기로 이전하고, 길드의 등급도 여기에 지정해야 해. 아직 사용하지 않는 헬퍼 함수는 구현 하지 마.
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/GameState/Components/GuildService.h / .cpp (신규)
GuildFunds: 길드 공통 지갑 잔액, 복제되며 쓰기는 서버 권위 (재사용: Source/Asteria/GameState/AsteriaGameState.h 에서 이전)
GuildRank: 길드 등급(ERank), 복제되며 기본값 F, 쓰기는 서버 권위 (신규)
UGuildService(): 컴포넌트 기본 복제 활성화, 틱 비활성화 (신규)
GetLifetimeReplicatedProps(): GuildFunds, GuildRank 복제 등록 (신규)
AddGuildFunds(): 잔액을 Amount만큼 늘림 - BTTask_WaitForSettleConfirm이 호출 (재사용: Source/Asteria/GameState/AsteriaGameState.cpp 에서 이전)
AddGuildFunds 검증: 오너에 권위가 없거나 0 이하 금액이면 false (재사용: Source/Asteria/GameState/AsteriaGameState.cpp 에서 이전, 추가)


[2단계]
Source/Asteria/GameState/AsteriaGameState.h / .cpp (수정)
GuildService: 길드 상태 컴포넌트, 생성자에서 기본 서브오브젝트로 생성 (신규)
GuildFunds: GuildService로 이전되어 삭제 (수정)
AddGuildFunds(): GuildService로 이전되어 삭제 (수정)
GetLifetimeReplicatedProps(): 복제할 멤버가 없어져 오버라이드 삭제 (수정)

Source/Asteria/BTNode/BTTask_WaitForSettleConfirm.cpp (수정)
FBTWaitForSettleConfirmMemory::GameState: 입금처를 GuildService 약참조로 교체 (수정)
ExecuteTask(): GameState의 GuildService를 찾아 메모리에 보관 - AAsteriaGameState::GuildService 참조 (수정)
ExecuteTask GuildService 검증: GuildService가 없으면 Failed (신규, 추가)
정산 람다: 정산 확정 시 수수료 입금 - UGuildService::AddGuildFunds() 호출 (수정)
OnTaskFinished(): GuildService 약참조 초기화 (수정)
