```yaml
요청: 프리뷰 단계에서 메시가 바라보는 앞 방향으로 데칼이 깔리도록
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

1단계:
  Source/Asteria/Player/AsteriaPlayer.h / .cpp (수정):
    PreviewDecalComp: 미리보기 메시의 앞(메시 기준 +Y) 바닥에 화살표를 깔아 앞 방향을 보여주는 데칼 컴포넌트. PreviewComp의 자식이라 위치·뒤집힘 회전을 그대로 따름. 데칼 재질(/Game/Materials/M_PreviewArrowDecal)은 블루프린트 컴포넌트 설정에서 지정 (신규)
    AAsteriaPlayer(): PreviewDecalComp 생성. 미리보기 앞쪽 바닥 위치, 아래로 투영하는 회전, 화살표가 앞을 가리키는 방향, 크기를 상대 트랜스폼으로 지정, 처음엔 숨김 (수정)
    UpdatePreview(): 미리보기를 보이거나 숨길 때 자식인 PreviewDecalComp도 함께 보이거나 숨김 (수정)
```
