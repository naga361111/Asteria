```yaml
요청: 플레이어가 광선을 만드는거 까지. 격자 관련해서는 미구현.
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

1단계:
  Source/Asteria/Player/AsteriaPlayer.h / .cpp (수정):
    GetViewRay(): 1인칭으로 바라보는 광선을 만듦. 출발점 = 카메라 월드 위치, 방향 = 카메라 정면(화면 중앙) - Tick()이 호출, CameraComp 사용 (신규)
    Tick(): 로컬 조종 중일 때만 GetViewRay()로 광선을 만들고 확인용 디버그 선을 한 프레임 그림(F8로 빠져나와 보면 보임). 격자 연결 때 이 자리에서 격자 함수를 호출하게 됨 - GetViewRay() 호출 (수정)
```
