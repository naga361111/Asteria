```yaml
요청: 메시 배치 로직만.
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

1단계:
  Source/Asteria/Building/BuildingGrid.cpp (수정):
    SpawnEdgeMeshes(): 인스턴스 트랜스폼을 변의 Transform(변 중점, 변 회전) 그대로 사용. 메시 피벗이 가로·두께 중심의 바닥에 있다는 전제로, 시작 꼭짓점으로 반 칸 옮기던 계산과 주석을 제거 (수정)
```
