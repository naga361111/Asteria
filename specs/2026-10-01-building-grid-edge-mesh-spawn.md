요청: FBuildingGridEdge의 mesh를 실제 위치에 스폰해야 해.
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

[1단계]
Source/Asteria/Building/BuildingGrid.h / .cpp (수정)
FBuildingGridEdge::Mesh: 주석에서 "표시는 아직 안 함" 제거 (수정)
OnConstruction(): CalculateEdges() 다음에 SpawnEdgeMeshes() 호출 - SpawnEdgeMeshes() 호출 (수정)
SpawnEdgeMeshes(): Mesh가 있는 변마다 그 메시의 인스턴스를 추가. 메시 종류마다 인스턴스드 스태틱 메시 컴포넌트 하나를 만들어 루트에 붙임. 인스턴스 위치는 변의 시작 꼭짓점(메시 피벗이 아래·왼쪽이고 +X로 뻗으므로), 회전은 변의 회전, 액터 기준 - OnConstruction()이 호출 (신규, 재사용: 엔진 Components/InstancedStaticMeshComponent.h UInstancedStaticMeshComponent)
SpawnEdgeMeshes() 컴포넌트 정리: 만든 컴포넌트를 컨스트럭션 스크립트 생성으로 표시해, OnConstruction이 다시 돌 때 엔진이 이전 컴포넌트를 지우고 레벨에는 함께 저장되게 함(쿠킹된 게임·PIE에서 OnConstruction이 다시 돌지 않아도 보임) (신규, 추가)
