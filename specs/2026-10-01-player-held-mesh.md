```yaml
요청: 잡을 수 있는 메시 목록, 선택된 메시 인덱스, 마우스 휠로 인덱스 조절. 이게 구현 스코프. 나머지는 안해
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

1단계:
  Source/Asteria/Player/AsteriaPlayer.h / .cpp (수정):
    PlaceableMeshes: 잡을 수 있는 메시 목록. 블루프린트 기본값에서 지정 (신규)
    SelectedMeshIndex: 선택된 메시의 PlaceableMeshes 인덱스. 시작은 0 (신규)
    SelectAction: 마우스 휠 입력 액션(1D 축). 블루프린트 기본값에서 지정 (신규, 추가)
    SetupPlayerInputComponent(): SelectAction을 SelectMesh()에 바인딩 (수정)
    SelectMesh(): 휠 방향에 따라 SelectedMeshIndex를 1 늘리거나 줄임. 목록 끝을 넘으면 반대쪽 끝으로 돌아감 - SetupPlayerInputComponent()가 바인딩 (신규)
    SelectMesh() 빈 목록 처리: PlaceableMeshes가 비어 있으면 무시 (신규, 추가)
```
