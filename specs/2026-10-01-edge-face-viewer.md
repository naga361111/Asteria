```yaml
요청: R 유지한 채로 (설치 메시의 앞면이 항상 플레이어 쪽을 향하도록, R은 그 위에서 수동으로 한 번 더 뒤집기)
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

1단계:
  Source/Asteria/Player/AsteriaPlayer.h / .cpp (수정):
    ShouldFlip(): 변과 시점 위치로 최종 뒤집힘 여부를 반환. 뒤집지 않은 메시의 앞(데칼이 깔리는 쪽, 메시 기준 -Y)이 시점 반대편을 향하면 뒤집고, 그 결과에 bPlaceFlipped를 한 번 더 적용 - UpdatePreview(), Place()가 호출 (신규)
    Tick(): 찾은 변과 함께 시선 광선의 출발점을 UpdatePreview()에 넘김 - UpdatePreview() 호출 (수정)
    UpdatePreview(): 미리보기 회전에 bPlaceFlipped 대신 ShouldFlip() 결과를 사용 - ShouldFlip() 호출 (수정)
    Place(): Grid->SetEdgeMesh()에 bPlaceFlipped 대신 시선 광선 출발점으로 구한 ShouldFlip() 결과를 넘김 - ShouldFlip() 호출 (수정)
```
