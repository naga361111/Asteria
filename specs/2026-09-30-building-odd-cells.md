요청: @Content/PCG/PCG_Building.uasset 에서, 파라미터 Width, Height를 설정할 때, 짝수값을 설정 못하도록. 오직 홀수만 설정 가능하도록 제약
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

[1단계]
PCG/BuildingLayout.h (수정)
BuildingLayout::MaxCells: 칸 수 상한을 32에서 볼륨 범위(0~9600)에 들어가는 가장 큰 홀수 31로 변경, 주석에 홀수임을 명시 - PCGBuildingSettings.cpp, GuildShellService.cpp가 참조 (수정)
BuildingLayout::IsValidCells(): 칸 수가 1~MaxCells 범위의 홀수인지 판정. 칸 수의 유일한 유효성 기준 - PCGBuildingSettings.cpp ExecuteInternal(), GuildShellService.cpp BeginPlay()·SetShellSize()가 호출 (신규)


[2단계]
PCG/PCGBuildingSettings.h / .cpp (수정)
UPCGBuildingSettings::Width / Height: 기본값 8을 홀수 9로 변경, 주석을 "1~MaxCells 홀수만, 아니면 노드 오류"로 수정 (수정)
FPCGBuildingElement::ExecuteInternal(): 칸 수를 Clamp로 자르던 것을 IsValidCells()로 검증하도록 변경. 짝수이거나 범위 밖이면 그래프에 오류(값 포함)를 표시하고 점 없이 끝냄 - BuildingLayout::IsValidCells() 호출 (수정)
칸 수 검증: 그래프 파라미터(디테일 패널 입력)가 짝수·범위 밖이면 건물을 만들지 않음 (수정, 추가)

GameState/Components/GuildShellService.h / .cpp (수정)
BeginPlay(): 레벨의 Width·Height가 IsValidCells()를 통과할 때만 ShellWidth·ShellHeight로 삼음. 통과 못하면 경고 로그를 남기고 0(미정) 유지 - BuildingLayout::IsValidCells() 호출 (수정)
SetShellSize(): 범위 검사를 IsValidCells()로 교체해 짝수를 거부(false), 경고 로그에 "1~MaxCells 홀수" 명시. 헤더 주석의 실패 조건을 "권위 없음/1~31 홀수 아님"으로 수정 - BuildingLayout::IsValidCells() 호출 (수정)


[3단계]
Content/PCG/PCG_Building.uasset (수정)
그래프 파라미터 Width / Height: 기본값 8을 9로 변경 (수정, 추가)

Content/Map/PCG.umap (수정)
GuildShellVolume 인스턴스 파라미터 Width / Height: 8을 9로 변경하고 PCG를 다시 생성해 저장 (수정, 추가)
