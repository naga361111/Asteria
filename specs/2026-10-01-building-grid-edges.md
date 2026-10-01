요청: 각 변의 위치를 계산하는 시스템과 시각적으로 표현하는 기능만. 지금 사용되지 않는 다른 기능은 미구현
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

[1단계]
Source/Asteria/Building/BuildingGrid.h / .cpp (신규)
ABuildingGrid: 레벨에 배치하는 격자 액터. 액터 트랜스폼이 격자 원점. 격자의 각 변 위치를 계산 (신규)
ABuildingGrid::CellSize: 한 칸(변 길이) 300 상수 (신규)
ABuildingGrid::GridSize: 가로(X)·세로(Y) 칸 수, EditAnywhere, 기본 (3, 2), ClampMin 1 (신규)
ABuildingGrid::Edges: 격자의 모든 변 위치(액터 기준). 변마다 변 중점 위치와 변 방향(yaw 0 = X축, 90 = Y축)을 담은 트랜스폼. 길이는 CellSize, 디테일 패널에서 읽기 전용 - CalculateEdges()가 채움 (신규)
ABuildingGrid::EdgeLines: 변을 그리는 에디터 전용 선 컴포넌트 - DrawEdges()가 채움 (재사용: 엔진 Components/LineBatchComponent.h ULineBatchComponent)
ABuildingGrid(): 루트 씬 컴포넌트 생성, 에디터 빌드에서만 EdgeLines 생성, Tick 끔 (신규)
OnConstruction(): 배치·이동·GridSize 변경 시 CalculateEdges() 호출, 에디터 빌드에서 DrawEdges() 호출 (신규)
CalculateEdges(): GridSize로 Edges를 다시 계산. 가로 변 GridSize.X×(GridSize.Y+1)개, 세로 변 (GridSize.X+1)×GridSize.Y개 (신규)
DrawEdges(): 에디터 전용. EdgeLines를 비우고 Edges마다 중점에서 변 방향 양쪽으로 CellSize/2씩, CellSize 길이 선을 액터 트랜스폼을 적용해 그림 (신규)
