요청: 설계 (에디터 전용 디버그 콘솔 명령으로 런타임에 PCG 건물 외곽 크기 변경, 서버 권위 → 클라 동기화)
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/Asteria.Build.cs (수정)
PrivateDependencyModuleNames: PCG 모듈 의존성 추가 - GuildShellService가 PCG 컴포넌트를 다시 생성할 때 필요 (수정, 추가)

Content/Map/PCG.umap (수정)
GuildShellVolume 생성 방식: 로드 시 자동 생성 대신 요청할 때 생성(Generate On Demand)으로 변경 - GuildShellService가 Generate 호출 (수정)
GuildShellVolume 태그: `GuildShellVolume` 추가 - GuildShellService가 찾음 (수정, 추가)
GuildShellVolume 범위: 최대 크기(32×32칸, 0~9600) 외곽선 전체를 여유 있게 덮도록 확대 — 범위 밖은 바닥이 잘림 (수정, 추가)
PlayerStart: 기본 외곽선 안쪽 가운데에 배치해 PIE 시작 시 홀 안에서 시작 (신규, 추가)
생성 확인: 변경 후 에디터에서 그래프 1회 실행해 바닥 64·벽 32·기둥 4개 유지 (추가)


[2단계]
Source/Asteria/GameState/Components/GuildShellService.h / .cpp (신규)
ShellWidth: 외곽 가로 칸 수, 기본 8, 쓰기는 서버 권위 - ReplicatedUsing=OnRep_ShellSize (신규)
ShellHeight: 외곽 세로 칸 수, 기본 8, 쓰기는 서버 권위 - ReplicatedUsing=OnRep_ShellSize (신규)
GuildShellService(): 컴포넌트 복제 켜고 틱 끔 (신규)
GetLifetimeReplicatedProps(): ShellWidth·ShellHeight 복제 등록 (신규)
SetShellSize(): 서버 전용으로 칸 수를 바꾸고 ApplyShellSize() 호출, 실패 시 false - AAsteriaPlayer::Server_SetShellSize가 호출 (신규)
권한 확인: 오너가 서버 권위가 아니면 거절 (신규, 추가)
입력 검증: 칸 수가 1~32 범위 밖이면 거절 — 32는 GuildShellVolume 범위와 맞춘 상한 (신규, 추가)
OnRep_ShellSize(): 클라에서 복제된 칸 수를 받아 ApplyShellSize() 호출 (신규)
ApplyShellSize(): 태그 `GuildShell` 액터의 스플라인을 (0,0)~(ShellWidth×300, ShellHeight×300) 닫힌 직선 사각형으로 바꾸고 태그 `GuildShellVolume` 액터의 PCG 컴포넌트를 강제 재생성 (신규)
대상 확인: 외곽선 스플라인이나 PCG 컴포넌트를 못 찾으면 경고 로그 후 중단 (신규, 추가)


[3단계]
Source/Asteria/GameState/AsteriaGameState.h / .cpp (수정)
GuildShellService: 외곽 크기 서비스 컴포넌트 멤버, 생성자에서 기본 하위 객체로 생성 - AAsteriaPlayer::Server_SetShellSize가 참조 (신규)


[4단계]
Source/Asteria/Player/AsteriaPlayer.h / .cpp (수정)
ShellSize(): 에디터 빌드 전용 콘솔 명령(Exec), 가로·세로 칸 수를 받아 Server_SetShellSize() 호출 (신규)
Server_SetShellSize(): 에디터 빌드 전용 클라→서버 경계, GameState의 GuildShellService->SetShellSize()로 전달만 함 (신규)
