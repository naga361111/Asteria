요청: 구현 내용을 구체적으로 설계해봐. (퀘스트를 3시간 예정 시각 ±30분에 3~4개씩 묶음 발행, 퀘스트마다 4~8시간 제한 시간, 만료되면 QuestPull에서 제거, NPC가 집은 퀘스트는 만료 없음)
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/Common/Quest.h (수정)
ExpireGameMinute: 퀘스트가 만료되는 게임 시각(GameClockService의 게임 분 단위). 발행 시 정해지고 FQuest와 함께 복제됨 - QuestService가 설정·검사 (신규)


[2단계]
Source/Asteria/GameState/Components/QuestService.h / .cpp (수정)
MinBatchSize / MaxBatchSize: 한 묶음에 발행되는 퀘스트 수 범위, 값 3~4 - 익명 네임스페이스 상수 (신규)
MinQuestLifetimeMinutes / MaxQuestLifetimeMinutes: 퀘스트 제한 시간 범위(게임 분), 값 240~480 - 익명 네임스페이스 상수 (신규)
BatchPeriodMinutes: 발행 예정 시각 간격(게임 분), 값 180 - 익명 네임스페이스 상수 (신규)
BatchJitterMinutes: 실제 발행이 예정 시각에서 앞뒤로 흔들리는 폭(게임 분), 값 30 - 익명 네임스페이스 상수 (신규)
QuestClockIntervalSeconds: 발행·만료를 확인하는 실제 시간 주기, 값 1초 - 익명 네임스페이스 상수 (신규)
NextBatchSlotMinute: 다음 발행 예정 시각(흔들림 없는 기준, 게임 분). 발행할 때마다 BatchPeriodMinutes씩만 전진해 오차가 누적되지 않음 - 서버 전용, 복제 안 함 (신규)
NextBatchMinute: 다음 실제 발행 시각. NextBatchSlotMinute ± BatchJitterMinutes에서 랜덤 - 서버 전용, 복제 안 함 (신규)
QuestClockTimer: 1초 반복 확인 타이머 핸들 - TickQuestClock() 바인딩 (신규)
BeginPlay(): 50개 생성 루프를 제거하고, 첫 묶음을 즉시 발행한 뒤 NextBatchSlotMinute·NextBatchMinute를 정하고 QuestClockTimer를 시작. 발행 후 OnQuestPullChanged 통지 - IssueQuestBatch() 호출, 월드 TimerManager로 TickQuestClock() 바인딩 (수정)
BeginPlay 권위 확인: 오너에 서버 권위가 없으면 발행·타이머를 시작하지 않음 (재사용: Source/Asteria/GameState/Components/QuestService.cpp)
BeginPlay 서비스 검증: GameState·GuildService에 더해 GameClockService가 없으면 경고 로그를 남기고 발행하지 않음 (수정, 추가)
IssueQuestBatch(): MinBatchSize~MaxBatchSize개 퀘스트를 QuestPull에 추가. 등급은 발행 시점의 길드 등급으로 RollQuestRank, 보상·수수료는 기존 규칙, ExpireGameMinute는 현재 게임 시각 + 제한 시간 랜덤. 통지는 호출자 책임 - BeginPlay()·TickQuestClock()이 호출, RollQuestRank() 호출 (신규)
RollQuestRank(): 길드 등급 기준 가중 추첨 - IssueQuestBatch()가 호출 (재사용: Source/Asteria/GameState/Components/QuestService.cpp)
TickQuestClock(): 현재 게임 시각이 NextBatchMinute를 넘었으면 묶음을 발행하고 NextBatchSlotMinute를 BatchPeriodMinutes만큼 전진시킨 뒤 NextBatchMinute를 다시 뽑음. 이어서 만료 시각이 지났고 집히지 않은 퀘스트를 QuestPull에서 제거. 하나라도 바뀌었으면 OnQuestPullChanged 통지 - QuestClockTimer가 호출, IssueQuestBatch()·IsQuestAssigned()·GameClockService::GetGameMinutes() 호출 (신규)
만료 제외: NPC가 집은(Assignment가 있는) 퀘스트는 만료 시각이 지나도 제거하지 않음 - IsQuestAssigned() 호출 (재사용: Source/Asteria/GameState/Components/QuestService.cpp)
GetGameMinutes(): 서버 월드 시간에서 파생한 현재 게임 분 - IssueQuestBatch()·TickQuestClock()이 호출 (재사용: Source/Asteria/GameState/Components/GameClockService.cpp)
