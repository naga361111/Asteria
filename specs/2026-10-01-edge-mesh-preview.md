```yaml
요청: 플레이어가 유효한 메시 인덱스를 가리키고 변 을 잡았을 때, 현재 잡은 메시에 Preview 머티리얼을 적용해서 미리보기를 띄우는 기능
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

1단계:
  Source/Asteria/Player/AsteriaPlayer.h / .cpp (수정):
    PreviewComp: 미리보기 메시를 보여주는 스태틱 메시 컴포넌트. 루트에 붙지만 월드 트랜스폼을 직접 지정(절대 트랜스폼), 충돌·그림자 끔, 처음엔 숨김 (신규)
    PreviewMaterial: 미리보기에 덮어쓸 반투명 재질. 블루프린트 기본값에서 /Game/Materials/M_BuildPreview 지정 (신규)
    AAsteriaPlayer(): PreviewComp 생성과 위 설정 (수정)
    Tick(): 찾은 변(못 찾았으면 없음)으로 매 프레임 UpdatePreview() 호출. Grid가 없거나 광선을 못 만들어도 없음으로 호출. 기존 로컬 조종 조건과 디버그 선은 유지 - UpdatePreview() 호출 (수정)
    UpdatePreview(): 변이 있고 SelectedMeshIndex가 PlaceableMeshes 범위 안이면, 선택된 메시를 PreviewComp에 넣고 모든 재질 슬롯에 PreviewMaterial을 적용한 뒤, 변 위치(설치될 위치와 동일)로 옮겨 보이게 함. 아니면 숨김 - Tick()이 호출 (신규)
    UpdatePreview() 재질 검증: PreviewMaterial이 지정되지 않았으면 원래 재질로 보이는 미리보기를 띄우지 않고 숨김 (신규, 추가)
```
