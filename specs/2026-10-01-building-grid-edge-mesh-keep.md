요청: 보존 기능만 구현
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

[1단계]
Source/Asteria/Building/BuildingGrid.h / .cpp (수정)
FBuildingGridEdge::Mesh: 이 변에 놓을 메시(UStaticMesh). null = 빈 변. 디테일 패널에서 편집. 표시(메시 생성)는 하지 않음 (신규, 추가)
ABuildingGrid::Edges: 원소의 Mesh를 디테일 패널에서 고를 수 있게 편집 가능으로 바꾸되 원소 추가·삭제는 막음. Vertex·Axis·Transform은 읽기 전용 유지 (수정, 추가)
CalculateEdges(): 다시 계산하기 전 기존 Edges를 따로 두고, 새로 만든 변마다 같은 (Vertex, Axis) 키의 기존 변이 있으면 Mesh를 옮겨 옴. 개수·순서·위치 계산은 기존과 같음 (수정)
CalculateEdges() 경고: 기존 변에 Mesh가 있었는데 새 Edges에 같은 키가 없으면(격자 축소로 버려짐) 키와 메시 이름을 LogTemp 경고로 남김 (수정, 추가)
