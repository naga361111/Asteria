요청: 배치 가능한 포인트들만 설계하는 시스템 구성
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

[1단계]
Source/Asteria/Building/BuildingGrid.h / .cpp (신규)
ABuildingGrid: 레벨에 배치하는 격자 액터. 액터 트랜스폼이 격자 원점. 격자의 포인트와 변만 생성 (신규)
ABuildingGrid::CellSize: 한 칸 300 상수 (신규)
ABuildingGrid::GridSize: 가로(X)·세로(Y) 칸 수, EditAnywhere, 기본 (3, 2), ClampMin 1 (신규)
ABuildingGrid::Points: 격자 꼭짓점 위치 목록(액터 기준), (GridSize.X+1)×(GridSize.Y+1)개 - GenerateGrid()가 채움 (신규)
ABuildingGrid::Edges: 이웃한 두 포인트를 잇는 변 목록, 변마다 (시작 포인트 인덱스, 끝 포인트 인덱스) - GenerateGrid()가 채움 (신규)
ABuildingGrid::PointLines: 포인트·변을 그리는 에디터 전용 선 컴포넌트 - DrawDebugGrid()가 채움 (재사용: 엔진 Components/LineBatchComponent.h ULineBatchComponent)
ABuildingGrid(): 루트 씬 컴포넌트 생성, 에디터 빌드에서만 PointLines 생성, Tick 끔 (신규)
OnConstruction(): 배치·이동·GridSize 변경 시 GenerateGrid() 호출, 에디터 빌드에서 DrawDebugGrid() 호출 (신규)
GenerateGrid(): GridSize로 Points와 Edges를 다시 만듦 (신규)
DrawDebugGrid(): 에디터 전용. PointLines를 비우고 Points를 점으로, Edges를 선으로 액터 트랜스폼을 적용해 그림 (신규)
