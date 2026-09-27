요청: 길드의 현 등급과, 명성 진행률을 보여주는 UI를 구현할꺼야. 얘도 HUD에서 불려들어가.
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/GameState/Components/GuildService.h / .cpp (수정)
FOnGuildReputationChanged: 길드 등급·명성이 바뀌었음을 알리는 인자 없는 멀티캐스트 델리게이트 타입. FOnGuildFundsChanged와 같은 방식 (신규, 추가)
GuildRank: 복제 방식을 Replicated에서 ReplicatedUsing=OnRep_GuildReputation으로 변경. 등급과 명성은 함께 바뀌므로 같은 OnRep 공유 (수정)
GuildReputation: 복제 방식을 Replicated에서 ReplicatedUsing=OnRep_GuildReputation으로 변경 (수정)
OnGuildReputationChanged: 등급·명성 변경 통지 델리게이트 멤버 - GuildRankWidget이 구독 (신규, 추가)
OnRep_GuildReputation(): 클라에서 등급 또는 명성이 복제되어 들어오면 OnGuildReputationChanged Broadcast (신규, 추가)
AddGuildReputation(): 명성 증가(및 승급) 성공 후 true를 반환하는 모든 경로(ReputationData 없음 경로 포함)에서 OnGuildReputationChanged Broadcast. 서버(호스트)는 OnRep이 자동 호출되지 않으므로 직접 통지 (수정)
GetReputationProgress(): 현 등급 도달 기준 → 다음 등급 기준 구간에서 누적 명성이 차지하는 비율(0~1). 서버·클라 양쪽에서 복제된 값과 ReputationData(기본값 에셋 참조)로 계산하는 파생 질의 - ReputationData의 ReputationToReachRank 조회 (신규)
GetReputationProgress 검증: 최고 등급(S)이면 1, ReputationData가 없거나 기준 항목이 없거나 구간 폭이 0 이하이면 0 반환, 결과는 0~1로 제한 (신규, 추가)


[2단계]
Source/Asteria/UI/HUD/GuildRankWidget.h / .cpp (신규)
RankText: 현 길드 등급을 표시할 텍스트 블록. BindWidget, WBP의 위젯 이름과 일치 (신규)
ReputationBar: 명성 진행률을 표시할 프로그레스 바. BindWidget, WBP의 위젯 이름과 일치 - 엔진 UProgressBar 사용 (신규)
NativeConstruct(): GameState의 GuildService.OnGuildReputationChanged에 RefreshRank 바인딩 후 초기 표시를 위해 한 번 호출. GuildFundsWidget::NativeConstruct와 같은 패턴 - GuildService의 OnGuildReputationChanged 구독 (신규)
RefreshRank(): GuildRank를 등급 이름으로 RankText에 표시(QuestTileEntryWidget의 StaticEnum<ERank> 표시 방식 재사용), GetReputationProgress() 값을 ReputationBar에 설정 - OnGuildReputationChanged에 의해 호출, GuildService의 GetReputationProgress() 호출 (신규)
RefreshRank 검증: RankText·ReputationBar·GameState·GuildService 중 하나라도 없으면 아무것도 하지 않음 (신규, 추가)
