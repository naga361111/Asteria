```yaml
요청: 배치할 때, 방향을 지정할 수 있어야 해. 180도 회전만 가능. 프리뷰에서도 반영되어야 해.
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

1단계:
  Source/Asteria/Building/BuildingGrid.h / .cpp (수정):
    FBuildingGridEdge::bFlip: true면 이 변의 메시를 yaw 180도로 놓음. 에디터에서 편집 가능 (신규)
    CalculateEdges(): 다시 계산할 때 같은 키의 기존 변에서 Mesh와 함께 bFlip도 옮겨 옴 (수정)
    GetEdgeMeshTransform(): 변과 뒤집힘 여부로 메시를 놓을 액터 기준 트랜스폼을 반환. 뒤집히면 변 Transform에 yaw 180도를 더함, 위치는 그대로. public static - SpawnEdgeMeshes(), AAsteriaPlayer::UpdatePreview()가 호출 (신규)
    SetEdgeMesh(): 변 키로 찾은 변에 Mesh와 함께 뒤집힘 여부도 할당 - AAsteriaPlayer::Place()가 호출 (수정)
    SpawnEdgeMeshes(): 인스턴스를 변의 bFlip을 반영한 GetEdgeMeshTransform() 결과로 배치 - GetEdgeMeshTransform() 호출 (수정)
  Source/Asteria/Player/AsteriaPlayer.h / .cpp (수정):
    bPlaceFlipped: 현재 설치 방향(뒤집힘 여부) 멤버 변수. 시작은 false (신규)
    FlipAction: 방향을 뒤집는 입력 액션. 블루프린트 기본값에서 지정 (신규, 추가)
    SetupPlayerInputComponent(): FlipAction 눌림(Started)을 FlipPlacement()에 바인딩 (수정)
    FlipPlacement(): bPlaceFlipped를 뒤집음 - SetupPlayerInputComponent()가 바인딩 (신규)
    UpdatePreview(): 미리보기 위치·회전을 bPlaceFlipped를 반영한 ABuildingGrid::GetEdgeMeshTransform() 결과에 격자 트랜스폼을 곱해 정함 - ABuildingGrid::GetEdgeMeshTransform() 호출 (수정)
    Place(): Grid->SetEdgeMesh()에 bPlaceFlipped를 함께 넘김 - ABuildingGrid::SetEdgeMesh() 호출 (수정)
```
