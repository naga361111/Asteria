요청: B방향으로. (파라미터를 가운데 칸 기준 한쪽 칸 수 N으로 받고 칸 수 = 2N+1로 계산해 짝수 칸이 나올 수 없게 함)
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

[1단계]
PCG/BuildingLayout.h / .cpp (수정)
BuildingLayout::MaxHalfCells: 한쪽 칸 수 상한 15(칸 수 31 = 9300, 볼륨 범위 0~9600 안) - PCGBuildingSettings.cpp, GuildShellService.cpp가 참조 (신규)
BuildingLayout::Mesh::HalfLeft / HalfRight: 두 칸 문이 없어져 삭제(주석 포함) (수정)
BuildingLayout::FBuilder::FWall::DoorCell: 문 칸 범위(FIntPoint DoorCells) 대신 문 칸 번호 하나, 문 없으면 -1 (수정)
BuildingLayout::FBuilder(HalfWidth, HalfHeight, Doors[4]): 한쪽 칸 수를 받아 칸 수 = 2N+1(항상 홀수)로 계산해 Width·Height에 저장 (수정)
DoorCells(): 짝수 칸 분기 삭제. 문은 항상 한 칸(가운데 칸 + Offset, 1~Cells−2), 3칸 미만 벽이나 bEnabled=false면 문 없음 - 생성자가 호출 (수정)
BuildWalls(): 두 칸 문(HalfLeft + Entrance + HalfRight) 분기 삭제, 문 칸에는 Entrance 하나 (수정)


[2단계]
PCG/BuildingLayout.h (수정)
BuildingLayout::MaxCells: 참조가 모두 MaxHalfCells로 바뀌어 삭제 (수정)

PCG/PCGBuildingSettings.h / .cpp (수정)
UPCGBuildingSettings::HalfWidth / HalfHeight: Width·Height 대신 가운데 칸을 뺀 한쪽 칸 수(칸 수 = 2N+1), 기본 4, PCG_Overridable, 노드 편집 범위 0~MaxHalfCells. 그래프 파라미터 HalfWidth·HalfHeight를 받음 (수정)
FPCGBuildingElement::ExecuteInternal(): 한쪽 칸 수를 0~BuildingLayout::MaxHalfCells로 잘라 FBuilder에 넘김 - BuildingLayout::FBuilder 호출 (수정)
한쪽 칸 수 범위 검증: 그래프 파라미터(외부 입력)를 0~MaxHalfCells로 자름 (수정, 추가)

GameState/Components/GuildShellService.h / .cpp (수정)
ShellHalfWidth / ShellHalfHeight: ShellWidth·ShellHeight 대신 한쪽 칸 수를 복제. 0이 유효값(1칸)이 되어 기본값을 -1(미정)로 바꿔 실제 값은 항상 복제·OnRep되게 함 (수정)
그래프 파라미터 이름 상수: Width·Height 대신 HalfWidth·HalfHeight (수정)
BeginPlay(): 레벨의 HalfWidth·HalfHeight를 0~MaxHalfCells로 잘라 초기값으로 삼음 (수정)
SetShellSize(): 인자를 한쪽 칸 수로 받고 0~MaxHalfCells 범위 밖이면 거부(false)·경고 로그 - OnRep_ShellSize()와 함께 ApplyShellSize() 호출 (수정)
ApplyShellSize(): HalfWidth·HalfHeight 그래프 파라미터를 비교·설정 (수정)
권위·범위 검증: 서버 권위 확인과 범위 검사는 그대로 유지 (수정, 추가)

Player/AsteriaPlayer.h / .cpp (수정)
ShellSize() / Server_SetShellSize(): 인자 이름과 콘솔 명령 주석을 한쪽 칸 수("ShellSize 가로반 세로반", 칸 수 = 2N+1)로 수정. 전달만 하고 검증은 서비스가 함 (수정)


[3단계]
Content/PCG/PCG_Building.uasset (수정)
그래프 파라미터 Width / Height: 삭제 (수정)
그래프 파라미터 HalfWidth / HalfHeight: int32, 기본 4(9칸) (신규)
Get Graph Parameter 2개: HalfWidth·HalfHeight를 읽어 Building 노드의 HalfWidth·HalfHeight 핀에 연결 (수정)

Content/Map/PCG.umap (수정)
GuildShellVolume 인스턴스 파라미터: HalfWidth 4, HalfHeight 4(9×9칸), 문 설정 유지, PCG 재생성 후 저장 (수정, 추가)
