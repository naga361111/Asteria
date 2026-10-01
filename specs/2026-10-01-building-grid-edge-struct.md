요청: (직전 검토안) Edges를 FTransform 배열에서 변 키(Vertex, Axis)와 Transform을 담은 구조체 배열로 변경. Module 필드·키로 모듈 보존은 모듈 배치 단계에서
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

[1단계]
Source/Asteria/Building/BuildingGrid.h / .cpp (수정)
FBuildingGridEdge: 격자 변 하나의 데이터 USTRUCT. 변마다 데이터를 붙이는 단위 - ABuildingGrid::Edges의 원소 (신규)
FBuildingGridEdge::Vertex: 변의 시작 꼭짓점 격자 좌표(FIntPoint). Axis와 함께 변 키, 디테일 패널에서 읽기 전용 (신규)
FBuildingGridEdge::Axis: 변 방향 0 = X축, 1 = Y축. Vertex와 함께 변 키, 디테일 패널에서 읽기 전용 (신규)
FBuildingGridEdge::Transform: 변 중점 위치와 방향(yaw 0 = X축, 90 = Y축), 액터 기준, 디테일 패널에서 읽기 전용 (신규)
ABuildingGrid::Edges: FTransform 배열에서 FBuildingGridEdge 배열로 변경 - CalculateEdges()가 채우고 DrawEdges()가 읽음 (수정)
CalculateEdges(): 변마다 Vertex·Axis 키와 Transform을 채움. 개수·순서·중점 위치는 기존과 같음 (수정)
DrawEdges(): 각 원소의 Transform으로 선을 그림 (수정)
