요청: 최적화 (개선점 6번: BuildDecor의 Side 분기를 AddOnWall로 통일, Openings를 생성자에서 계산해 BuildWalls 순서 의존 제거. 깃발은 벽 기준 회전으로 바꿔 B면이 180° 돎)
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

[1단계]
GuildShellBuilder.h / .cpp (수정)
FWall::Openings: 문·창 구간. 생성자에서 채운다는 것을 주석에 반영 (수정)
DoorCells(): 벽 칸 수와 문 중심으로 문이 차지하는 첫·끝 칸을 구함(문 없으면 -1). BuildWalls의 First/Last 계산을 옮김 - 생성자, BuildWalls가 호출 (신규, cpp 익명 네임스페이스)
IsWindowCell(): 양 끝에서 센 번호가 홀수인 칸인지(창 벽 자리) - 생성자, BuildWalls가 호출 (신규, cpp 익명 네임스페이스)
FGuildShellBuilder(): 벽마다 DoorAlong을 정한 뒤 창 칸·문 구간으로 Openings를 채움 - DoorCells(), IsWindowCell() 호출. Build* 호출 순서는 그대로 (수정)
OnWall(): 벽 Side의 Along·Lateral 위치를 건물 좌표(U, V)로 바꿈. AddOnWall의 위치 계산을 옮김 - AddOnWall, BuildDecor가 호출 (신규)
AlongWall(): 건물 좌표 U, V를 벽 Side 시작에서 잰 진행 방향 거리로 바꿈(OnWall의 역) - BuildDecor가 호출 (신규)
AddOnWall(): Pitch 인자를 받아 Add에 넘김(기본 0, 기존 호출 그대로) - OnWall() 호출 (수정)
BuildWalls(): Openings를 채우지 않고 문 칸·창 칸 판정만 사용 - DoorCells(), IsWindowCell() 호출 (수정)
IsOpen(): 생성자에서 채운 Openings를 읽음 (재사용: Source/Asteria/Building/GuildShellBuilder.cpp)


[2단계]
GuildShellInterior.cpp (수정)
BuildDecor(): Side == 0 ? … : … 분기를 없애고 면 공통 코드로 배치. 결과 배치(위치·회전·스케일)는 깃발 회전을 빼고 현재와 같아야 함 (수정)
BuildDecor() 긴 벽 기둥·버팀대·벽 등: Along은 AlongWall(), 버팀대·벽 등은 AddOnWall(YawOffset 90, 버팀대는 Pitch 45·길이 스케일), 기둥 위치와 벽 등 조명 위치는 OnWall() - AlongWall(), AddOnWall(), OnWall(), Beam(), AddLight() 호출 (수정)
BuildDecor() 깃발: AddOnWall(YawOffset 0)로 벽 기준 배치. A면은 지금과 같고 B면은 180° 돎 - AlongWall(), AddOnWall() 호출 (수정)
BuildDecor() 박공 벽 등: 벽 1·3을 같은 코드로 돌고, Along 값 집합 {SconceV, Span - SconceV}를 직접 씀. 벽 등은 AddOnWall(YawOffset 90), 조명은 OnWall() - AddOnWall(), OnWall(), AddLight(), IsOpen() 호출 (수정)
