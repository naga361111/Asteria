요청: 기존 로직에서 알고리즘을 가져오는 형태. 완성되면 기존 로직은 지워지니까, 직접 불러오지 마. 벽과, 바닥을 PCG로 생성하는 로직을 Cpp와 PCG 에셋으로 설계.
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

[1단계]
PCG/BuildingLayout.h / .cpp (신규)
FBuildingDoor: 한 변의 문 설정 USTRUCT(bEnabled, Offset). 그래프 파라미터 DoorNegY·DoorPosX·DoorPosY·DoorNegX의 타입으로 디테일 패널에서 변마다 묶어 편집 - PCG_Building 그래프 파라미터가 사용 (신규, 알고리즘 출처: Building/GuildShellDoor.h)
BuildingLayout::Cell: 격자 한 칸 300 (신규)
BuildingLayout::MaxCells: 칸 수 상한 32. 볼륨 범위(0~9600)에 맞춘 값 - PCGBuildingSettings.cpp, GuildShellService.cpp가 참조 (신규)
BuildingLayout::CornerInset: 모서리 기둥을 벽 두께(40) 가운데에 놓는 외곽선 바깥 거리 20 (신규)
BuildingLayout::Mesh::Floor: SM_Floor_Tiles_300 (피벗 모서리, +X·-Y로 뻗음) (신규)
BuildingLayout::Mesh::Wall: SM_Wall_Tavern_D (300×300, 피벗 아래·왼쪽, +X로 뻗고 두께 -Y) (신규)
BuildingLayout::Mesh::WindowWall: SM_Wall_Tavern_D_Window (-Y면이 돌. 벽과 같은 yaw로 놓아 돌이 바깥을 봄) (신규)
BuildingLayout::Mesh::Entrance: SM_Wall_Tavern_D_Entrance_300 (문틀 포함 한 칸 문) (신규)
BuildingLayout::Mesh::HalfLeft / HalfRight: SM_Wall_Tavern_D_Half_lt / _rt (150폭, 짝수 칸 벽 가운데 문 양옆) (신규)
BuildingLayout::Mesh::Corner: SM_Wall_House_A_Column_B (40×40×300, 중심 피벗, D 세트에 모서리 부품이 없어 이음새를 가림) (신규)
BuildingLayout::FBuilder::Transforms / Meshes: 결과 배치 목록(점마다 트랜스폼과 메시 경로), 건물 로컬 좌표 - PCGBuildingSettings.cpp가 읽음 (신규)
BuildingLayout::FBuilder::FWall: 외곽 벽 하나(Start, Dir, Yaw, Cells, 문 칸 범위). 위에서 볼 때 시계 방향, 실내는 진행 방향 기준 로컬 +Y. 순서는 월드 변 NegY(yaw 0), PosX(90), PosY(180), NegX(-90) (신규, 알고리즘 출처: Building/GuildShellBuilder.h FWall)
BuildingLayout::FBuilder::Width / Height / Walls[4]: 칸 수와 네 벽 (신규)
BuildingLayout::FBuilder(Width, Height, Doors[4]): 네 벽을 세우고 문 칸을 정한 뒤 BuildFloor(), BuildWalls(), BuildCorners() 호출. Doors 순서는 월드 변 NegY, PosX, PosY, NegX (신규)
DoorCells(): 벽 칸 수와 문 설정으로 문이 차지하는 첫·끝 칸을 구함. 홀수 칸 벽은 가운데+Offset 한 칸(1~Cells−2), 짝수 칸 벽은 가운데 경계+Offset 양옆 두 칸(경계 2~Cells−2), 모서리 칸에 닿으면 멈춤, 둘 곳이 없는 짧은 벽(홀수 3칸 미만, 짝수 4칸 미만)이나 bEnabled=false면 문 없음. Offset 양수 = 실내에서 벽을 볼 때 오른쪽 - 생성자가 호출 (신규, 알고리즘 출처: Building/GuildShellBuilder.cpp DoorCenter·DoorCells)
IsWindowCell(): 양 끝에서 센 번호가 홀수인 칸이면 창(좌우 대칭) - BuildWalls()가 호출 (신규, 알고리즘 출처: Building/GuildShellBuilder.cpp IsWindowCell)
BuildFloor(): 칸마다 Floor를 무늬 방향이 돌지 않게 월드 축 기준으로 배치 - Add() 호출 (신규, 알고리즘 출처: Building/GuildShellBuilder.cpp BuildBase)
BuildWalls(): 벽마다 문 칸 밖은 창 칸이면 WindowWall, 아니면 Wall. 문은 한 칸이면 Entrance 하나, 두 칸이면 HalfLeft + Entrance(가운데) + HalfRight로 두 칸을 채움. 모든 조각은 벽 yaw 그대로(돌 면·두께가 바깥) - AddOnWall() 호출 (신규)
BuildCorners(): 네 모서리에 Corner를 이 벽과 앞 벽의 두께 가운데선이 만나는 점(외곽선 바깥 CornerInset)에 배치 - Add() 호출 (신규, 알고리즘 출처: Building/GuildShellBuilder.cpp BuildBase 모서리 기둥)
AddOnWall(): 벽 Side의 시작에서 진행 방향 거리 Along 위치에 벽 yaw로 메시 하나 추가 - Add() 호출 (신규)
Add(): Transforms·Meshes에 점 하나 추가 (신규)


[2단계]
PCG/PCGBuildingSettings.h / .cpp (신규)
UPCGBuildingSettings::Width / Height: 가로(X)·세로(Y) 칸 수, PCG_Overridable. 그래프 파라미터 Width·Height를 받음 (신규)
UPCGBuildingSettings::bDoorNegY / DoorNegYOffset / bDoorPosX / DoorPosXOffset / bDoorPosY / DoorPosYOffset / bDoorNegX / DoorNegXOffset: 변마다 문 설정, PCG_Overridable. 그래프 파라미터 DoorNegY.bEnabled 등을 받음 (신규)
GetDefaultNodeName() / GetDefaultNodeTitle() / GetNodeTooltipText() / GetType(): 에디터 노드 이름 "Building", 제목 "Building", 툴팁, Spatial (신규)
InputPinProperties(): 입력 핀 없음 (신규)
OutputPinProperties(): 출력 핀 Out 하나(점, Mesh 속성 포함) (신규)
CreateElement(): FPCGBuildingElement 생성 (신규)
FPCGBuildingElement::IsCacheable(): 항상 false. 결과가 볼륨 트랜스폼에 따라 달라지는데 입력 핀이 없어 캐시 키에 안 잡힘 (신규, 추가)
FPCGBuildingElement::ExecuteInternal(): 칸 수를 1~BuildingLayout::MaxCells로 자르고, FBuildingDoor 4개를 만들어 BuildingLayout::FBuilder 실행, 모든 점에 건물 원점 트랜스폼을 곱해 Out으로 출력 - BuildingLayout.h, GetBuildingOrigin(), MakePointData() 호출 (신규)
칸 수 범위 검증: 그래프 파라미터(외부 입력)를 1~MaxCells로 자름 (신규, 추가)
GetBuildingOrigin(): 건물 (0,0,0) = 실행 소스(볼륨)의 로컬 경계 최소 모서리. 볼륨 위치·회전을 반영하고 스케일은 빼 메시 크기를 유지 - Context->ExecutionSource의 실행 상태 트랜스폼·로컬 경계 사용 (신규, 추가)
MakePointData(): 트랜스폼·메시 경로 배열로 점 데이터를 만들고 점마다 Mesh 속성(FSoftObjectPath)을 담 (신규, 알고리즘 출처: Building/PCGGuildShellSettings.cpp MakePointData)

GameState/Components/GuildShellService.cpp (수정)
MaxShellCells: UPCGGuildShellSettings::MaxCells 대신 BuildingLayout::MaxCells 참조, include를 PCG/BuildingLayout.h로 교체 (수정, 추가)
ShellWidthParam / ShellHeightParam: 읽고 쓰는 그래프 파라미터 이름을 Width·Height로 교체 (수정, 추가)


[3단계]
Content/PCG/PCG_Building.uasset (신규)
그래프 파라미터 Width / Height: int32, 기본 8. GuildShellService가 이 이름으로 읽고 씀 (신규)
그래프 파라미터 DoorNegY / DoorPosX / DoorPosY / DoorNegX: FBuildingDoor (신규)
Get Graph Parameter ×10: Width, Height, DoorNegY.bEnabled, DoorNegY.Offset … DoorNegX.Offset를 읽음 - Building 노드의 같은 이름 핀에 연결 (신규)
Building 노드: UPCGBuildingSettings - Out을 Static Mesh Spawner에 연결 (신규)
Static Mesh Spawner: By Attribute(Mesh)로 메시 배치, 충돌 있음 - Output 노드에 연결 (신규)

Content/Map/PCG.umap (수정)
GuildShellVolume: 그래프를 /Game/PCG/PCG_Building으로 교체, 태그 GuildShellVolume 유지(GuildShellService가 이 태그로 찾음) (수정)
GuildShellVolume 트랜스폼: 스케일 (48, 48, 2)로 바꿔 위치 (4800, 4800, 200)에서 로컬 경계 최소 모서리가 월드 (0, 0, 0)이 되게 함. 건물이 기존과 같은 자리에 섬 (수정, 추가)
GuildShellVolume 인스턴스 파라미터: Width 8, Height 8, DoorNegY(bEnabled true, Offset 0), 나머지 문 bEnabled false (수정, 추가)
