요청: 위에서 설명한 기능의 구현 내용을 구체화. (로컬 플레이어별 HUD — AHUD가 메인 HUD 위젯을 띄우고, 메인 HUD 안에 길드 잔액 UI·게임 시간 UI를 각각 별도 위젯으로 배치. 각 UI는 GameState에서 직접 읽음)
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/GameState/Components/GuildService.h / .cpp (수정)
FOnGuildFundsChanged: 길드 잔액이 바뀌었음을 알리는 인자 없는 멀티캐스트 델리게이트 타입. QuestService.h의 FOnQuestPullChanged와 같은 방식 (신규, 추가)
GuildFunds: 복제 방식을 Replicated에서 ReplicatedUsing=OnRep_GuildFunds로 변경 (수정)
OnGuildFundsChanged: 잔액 변경 통지 델리게이트 멤버 - GuildFundsWidget이 구독 (신규, 추가)
OnRep_GuildFunds(): 클라에서 잔액이 복제되어 들어오면 OnGuildFundsChanged Broadcast (신규, 추가)
AddGuildFunds(): 잔액 증가 성공 후 OnGuildFundsChanged Broadcast. 서버(호스트)는 OnRep이 자동 호출되지 않으므로 직접 통지 (수정)

Source/Asteria/UI/HUD/GameClockWidget.h / .cpp (신규)
ClockText: 게임 시간을 표시할 텍스트 블록. BindWidget, WBP의 위젯 이름과 일치 (신규)
NativeTick(): 매 프레임 GameState의 GameClockService에서 일/시/분을 읽어 ClockText에 "N일차 HH:MM" 형식으로 표시. 시간은 복제 이벤트가 없는 파생값이라 폴링 - GameClockService의 GetDay()·GetHour()·GetMinute() 호출 (신규)
NativeTick 검증: ClockText·GameState·GameClockService 중 하나라도 없으면 아무것도 하지 않음 (신규, 추가)

Source/Asteria/UI/HUD/AsteriaHUD.h / .cpp (신규)
MainHUDClass: 화면에 띄울 메인 HUD 위젯 클래스(WBP_MainHUD). EditDefaultsOnly, BP에서 지정 (신규)
BeginPlay(): GameState가 이미 있으면 CreateMainHUD() 호출, 없으면(클라 초기 복제 전) 월드의 GameStateSetEvent에 CreateMainHUD 바인딩 - UWorld::GameStateSetEvent 사용(재사용: 엔진 World.h) (신규)
CreateMainHUD(): MainHUDClass 위젯을 소유 플레이어 컨트롤러로 생성해 뷰포트에 추가. AHUD는 로컬 플레이어에게만 생성되므로 플레이어별 화면 분리는 엔진이 보장 (신규)
CreateMainHUD 검증: MainHUDClass가 비어 있으면 경고 로그 후 중단 (신규, 추가)


[2단계]
Source/Asteria/UI/HUD/GuildFundsWidget.h / .cpp (신규)
FundsText: 길드 잔액을 표시할 텍스트 블록. BindWidget, WBP의 위젯 이름과 일치 (신규)
NativeConstruct(): GameState의 GuildService.OnGuildFundsChanged에 RefreshFunds 바인딩 후 초기 표시를 위해 한 번 호출. QuestBoardWidget::NativeConstruct와 같은 패턴 - GuildService의 OnGuildFundsChanged 구독 (신규)
RefreshFunds(): GuildService의 GuildFunds를 읽어 FundsText에 표시 - OnGuildFundsChanged에 의해 호출 (신규)
RefreshFunds 검증: FundsText·GameState·GuildService 중 하나라도 없으면 아무것도 하지 않음 (신규, 추가)
