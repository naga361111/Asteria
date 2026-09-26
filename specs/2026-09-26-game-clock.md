요청: 게임 시간은 일/시간/분 으로 정해져있어. 얘는 GameState 에 이거 전용 컴포넌트를 만들어서 기능을 다뤄. 일단 시스템상으로 시간만 만들어놔
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/GameState/Components/GameClockService.h / .cpp (신규)
GameMinutesPerRealSecond: 현실 1초당 흐르는 게임 분. .cpp 익명 네임스페이스 상수, QuestService.cpp의 튜닝 상수와 같은 방식 (신규, 추가)
StartGameMinutes: 게임 시작 시각(1일차 몇 시)을 게임 분으로 나타낸 상수. 위와 같은 방식 (신규, 추가)
UGameClockService(): 복제할 상태가 없으므로 복제·Tick 끔. 게임 시간은 GameState가 이미 복제하는 서버 월드 시간에서 파생 (신규)
GetGameMinutes(): 게임 시작부터 흐른 총 게임 분. 서버·클라 양쪽에서 같은 값 - 오너 GameState의 GetServerWorldTimeSeconds() 호출(재사용: 엔진 AGameStateBase) (신규)
GetGameMinutes 오너 검증: 오너가 GameState가 아니면 0 반환 (신규, 추가)
GetDay(): 현재 일. 1일부터 시작 - GetGameMinutes() 호출 (신규)
GetHour(): 현재 시(0~23) - GetGameMinutes() 호출 (신규)
GetMinute(): 현재 분(0~59) - GetGameMinutes() 호출 (신규)


[2단계]
Source/Asteria/GameState/AsteriaGameState.h / .cpp (수정)
GameClockService: 게임 시간 컴포넌트, 기존 QuestService·CounterService·GuildService와 같은 방식으로 선언 - UGameClockService 참조 (신규)
AAsteriaGameState(): GameClockService를 기본 서브오브젝트로 생성 (수정)
