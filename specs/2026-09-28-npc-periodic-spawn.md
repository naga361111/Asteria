요청: Npc 스폰 기능 구체화.
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/GameState/Components/QuestService.h / .cpp (수정)
CountAvailableQuests(): 아직 안 집혔고 등급이 MaxRank 이하인 퀘스트 수. FindAvailableQuestId와 같은 필터(천장 + 이미 집힘)로 "이 등급 NPC가 집을 수 있는 퀘스트가 몇 개인가"를 셈. 파생 질의 구역에 둠 - NpcSpawner::TickSpawnClock()이 호출, IsQuestAssigned() 호출 (신규)
IsQuestAssigned(): 퀘스트에 살아있는 Assignment가 있는지 - CountAvailableQuests()가 호출 (재사용: Source/Asteria/GameState/Components/QuestService.cpp)
IsNpcAssigned(): NpcId가 파티에 든 Assignment가 하나라도 있는지(상태 무관). 스폰됐지만 아직 퀘스트를 못 집은 NPC를 가려내는 용도. 파생 질의 구역에 둠 - NpcSpawner::TickSpawnClock()이 호출 (신규, 추가)


[2단계]
Source/Asteria/Actors/NpcSpawner.h / .cpp (수정)
SpawnCheckIntervalSeconds: 스폰 여부를 확인하는 실제 시간 주기, 값 1초 - 익명 네임스페이스 상수 (신규)
SpawnRateMinutesPerQuest: 방문 확률 기준값 K(게임 분). 1분당 방문 확률 ≈ 집을 수 있는 퀘스트 수 ÷ K, 값 120. 퀘스트 공급(180분에 3~4개)과 균형이 맞아 게시판이 평소 2~3개로 유지되는 값 - 익명 네임스페이스 상수 (신규)
MinSpawnGapMinutes: 연속 스폰 사이 최소 간격(게임 분), 값 5. 스폰 지점에서 NPC가 겹쳐 나오는 것을 막음 - 익명 네임스페이스 상수 (신규)
NextNpcId: 스폰마다 고유 NpcId를 발급하는 카운터. 0은 에디터 기본값이라 1부터 발급. 스포너가 여러 개여도 겹치지 않도록 인스턴스가 아닌 파일 범위에 둠 - 익명 네임스페이스 변수, SpawnNpc()가 사용 (신규)
SpawnTimer: SpawnCheckIntervalSeconds 반복 확인 타이머 핸들 - TickSpawnClock() 바인딩 (신규)
LastCheckMinute: 직전 확인 시각(게임 분). 확인 사이 흐른 게임 분만큼 확률을 적용하는 기준 - 서버 전용 (신규)
LastSpawnMinute: 마지막 스폰 시각(게임 분). 시작 직후 바로 스폰할 수 있는 값으로 초기화 - 서버 전용 (신규)
BeginPlay(): BeginPlay에서 1회 스폰하던 것을 제거하고, LastCheckMinute·LastSpawnMinute를 현재 게임 시각 기준으로 정한 뒤 SpawnTimer를 시작 - 월드 TimerManager로 TickSpawnClock() 바인딩, GameClockService::GetGameMinutes() 호출 (수정)
BeginPlay 권위 확인: 서버 권위가 없거나 NpcClass가 없으면 타이머를 시작하지 않음 (재사용: Source/Asteria/Actors/NpcSpawner.cpp)
BeginPlay 서비스 검증: AsteriaGameState·QuestService·GameClockService가 없으면 경고 로그를 남기고 타이머를 시작하지 않음 (신규, 추가)
TickSpawnClock(): 현재 게임 시각과 LastCheckMinute로 흐른 게임 분을 구해 LastCheckMinute를 갱신. 아래 조건을 모두 통과하면 1 − e^(−집을 수 있는 퀘스트 수 × 흐른 분 ÷ K) 확률로 SpawnNpc()를 호출하고 LastSpawnMinute를 갱신. 간격이 고정되지 않고 퀘스트가 많을수록 자주, 없으면 오지 않음 - SpawnTimer가 호출, GameClockService::GetGameMinutes()·SpawnNpc() 호출 (신규)
인원 상한 확인: 월드의 AAsteriaNpc 수가 PointType이 Lounge인 AApproachPointActor 수 이상이면 스폰하지 않음. NPC는 한 자리만 가지므로 NPC 수 < 자리 수면 빈 자리가 반드시 있음 - TActorIterator로 셈 (신규, 추가)
최소 간격 확인: 현재 게임 시각 − LastSpawnMinute가 MinSpawnGapMinutes 미만이면 스폰하지 않음 (신규)
집을 수 있는 퀘스트 수: NpcClass 기본 등급(NpcRnk) 기준 CountAvailableQuests()에서, 월드의 NPC 중 IsNpcAssigned()가 false인(아직 퀘스트를 못 집은) 수를 뺀 값. 0 이하면 스폰하지 않음 — 이미 보드로 향하는 NPC 몫의 퀘스트를 중복으로 세지 않아 도착해서 못 집는 경우를 막음 - QuestService::CountAvailableQuests()·IsNpcAssigned() 호출 (신규, 추가)
GetGameMinutes(): 서버 월드 시간에서 파생한 현재 게임 분 - BeginPlay()·TickSpawnClock()이 호출 (재사용: Source/Asteria/GameState/Components/GameClockService.cpp)
SpawnNpc(): 스포너 위치·회전에 NpcClass를 지연 스폰해 BeginPlay·빙의(BT 시작) 전에 NpcId를 NextNpcId로 부여한 뒤 스폰을 마치고, 컨트롤러가 없으면 AI 컨트롤러를 부여. 충돌 처리는 기존대로 AdjustIfPossibleButAlwaysSpawn - TickSpawnClock()이 호출, APawn::SpawnDefaultController() 호출 (신규)
