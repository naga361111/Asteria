```yaml
요청: 클릭 입력 시 찾은 Edges의 메시를 플레이어에서 선택된 메시로 할당하는 기능
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

1단계:
  Source/Asteria/Building/BuildingGrid.h / .cpp (수정):
    SetEdgeMesh(): 변 키(Vertex, Axis)로 Edges에서 변을 찾아 Mesh를 할당하고, 이 격자의 기존 인스턴스 메시 컴포넌트를 모두 지운 뒤 SpawnEdgeMeshes()로 다시 만듦. public - AAsteriaPlayer::Place()가 호출, SpawnEdgeMeshes() 호출 (신규)
    SetEdgeMesh() 키 검증: 키에 해당하는 변이 Edges에 없으면 아무것도 하지 않음 (신규, 추가)
    SpawnEdgeMeshes(): Edges의 메시마다 인스턴스 메시 컴포넌트를 만들어 배치 - OnConstruction(), SetEdgeMesh()가 호출 (재사용: Source/Asteria/Building/BuildingGrid.cpp)

2단계:
  Source/Asteria/Player/AsteriaPlayer.h / .cpp (수정):
    PlaceAction: 클릭 입력 액션. 블루프린트 기본값에서 지정 (신규, 추가)
    SetupPlayerInputComponent(): PlaceAction 눌림(Started)을 Place()에 바인딩 (수정)
    Place(): GetViewRay()의 광선으로 Grid->FindNearestEdge()를 호출해 찾은 변에 PlaceableMeshes의 SelectedMeshIndex 메시를 Grid->SetEdgeMesh()로 할당 - SetupPlayerInputComponent()가 바인딩, GetViewRay(), ABuildingGrid::FindNearestEdge(), ABuildingGrid::SetEdgeMesh() 호출 (신규)
    Place() 실패 처리: Grid가 없거나, 광선을 못 만들거나, 변을 못 찾거나, SelectedMeshIndex가 PlaceableMeshes 범위 밖(빈 목록 포함)이면 무시 (신규, 추가)
```
