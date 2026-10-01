```yaml
요청: 조준점까지는 내가 구현해두었어.
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

1단계:
  Source/Asteria/Player/AsteriaPlayer.h / .cpp (수정):
    GetViewRay(): 광선이 항상 화면 중앙 조준점(WBP_PlayerHUD)을 지나도록, 카메라 정면 대신 뷰포트 중앙 픽셀을 월드로 역투영해 광선을 만듦. 성공 여부를 반환하고 광선은 출력 인자로 넘김 - Tick()이 호출, 재사용: 엔진 APlayerController::GetViewportSize / DeprojectScreenPositionToWorld (수정)
    GetViewRay() 실패 처리: 플레이어 컨트롤러가 없거나 역투영이 실패하면(서버, 초기화 전) 실패 반환 (수정, 추가)
    Tick(): GetViewRay()가 실패하면 디버그 선을 그리지 않고 건너뜀. 로컬 조종 조건과 디버그 선은 유지 - GetViewRay() 호출 (수정)
```
