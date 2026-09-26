요청: 이제 GameState 에서 퀘스트를 발행할 때, 위에서 만든 표 대로, 등급이 부여되서 발행되도록
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/GameState/Components/QuestService.h / .cpp (수정)
LowerRankFalloff: 길드 등급에서 한 단계 내려갈 때마다 곱하는 무게 비율, 값 0.5 - 익명 네임스페이스 상수 (신규)
UpperRankWeight: 길드 등급 바로 위 한 단계의 무게, 값 0.3 - 익명 네임스페이스 상수 (신규)
RollQuestRank(): 길드 등급 기준으로 등급 하나를 가중 추첨. 같은 등급 무게 1, 아래 모든 등급은 단계마다 LowerRankFalloff 배, 위 한 단계만 UpperRankWeight, 그보다 위는 제외. S급 초과는 존재하지 않으므로 제외 - BeginPlay()가 호출 - 익명 네임스페이스 함수 (신규)
UQuestService(): 퀘스트 생성 루프를 제거하고 복제 설정만 남김 (수정)
BeginPlay(): 서버에서 퀘스트 50개 생성. 등급은 RollQuestRank(GuildService의 GuildRank)로 정하고 보상·수수료는 기존 규칙 유지. 생성 후 OnQuestPullChanged 통지 - AAsteriaGameState::GuildService, RollQuestRank() 참조 (신규, 추가)
BeginPlay 권위 확인: 오너에 서버 권위가 없으면 생성하지 않음. 클라는 복제된 QuestPull만 받음 (신규, 추가)
BeginPlay GuildService 검증: 오너 GameState나 GuildService가 없으면 경고 로그를 남기고 생성하지 않음 (신규, 추가)
