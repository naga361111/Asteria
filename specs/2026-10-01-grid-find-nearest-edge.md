```yaml
요청: Ray 로 맞춘 부분에서 가장 가까운 그리드 정보를 가져와야 해.
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

1단계:
  Source/Asteria/Building/BuildingGrid.h / .cpp (수정):
    FindNearestEdge(): 광선이 격자 바닥 평면(액터 위치를 지나고 액터 위쪽에 수직)과 만나는 점에서 가장 가까운 변을 찾아 반환. 만난 점을 액터 기준 칸 좌표로 바꾼 뒤, 가로 변 후보와 세로 변 후보 중 Edges에 있는 것 가운데 더 가까운 쪽. public, const - AAsteriaPlayer::Tick()이 호출 (신규)
    FindNearestEdge() 실패 처리: 광선이 평면과 평행이거나 만나는 점이 광선 뒤쪽이면, 또는 두 후보 모두 Edges에 없으면(격자 밖) 없음을 반환 (신규, 추가)

2단계:
  Source/Asteria/Player/AsteriaPlayer.h / .cpp (수정):
    Grid: 변을 찾을 격자를 가리키는 약한 참조 멤버 변수. 레벨의 첫 번째 ABuildingGrid - BeginPlay()가 설정 (신규, 추가)
    BeginPlay(): 레벨에서 첫 번째 ABuildingGrid를 찾아 Grid에 저장 - TActorIterator 사용 패턴 재사용: Source/Asteria/GameState/Components/NpcSpawnService.cpp (수정)
    Tick(): 광선 확인용 디버그 선을 지우고, Grid가 유효하면 GetViewRay()의 광선으로 Grid->FindNearestEdge()를 호출해 찾은 변을 월드 좌표의 디버그 선(변 길이)으로 한 프레임 그림. 로컬 조종 조건은 유지 - GetViewRay(), ABuildingGrid::FindNearestEdge() 호출 (수정)
```
