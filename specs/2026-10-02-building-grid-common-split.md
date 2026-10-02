요청: 에디터는 무시하고, 현재 구현된 GridBuilding을 Common이랑 1인칭 용이랑 분리하는 리팩토링 계획. 절대 기존 기능에서 변하는게 없어야 해. 오직 구현 수준에서만 갈라지는 변화가 생기는거야. (추가 요청 1) FirstPerson에서는 이름을 보편적으로 하지 말고 스코프에 맞게 작성. 보편적인 이름은 Common만. (추가 요청 2) 호출 체인은 최대한 짧게. (추가 요청 3) 공통 로직은 싹 다 Common에, FirstPerson에서만 특수하게 쓰는 로직만 FirstPerson 영역에.
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

## 분류
- Common: ABuildingGrid 전체 — CellSize, GridSize, Edges, FBuildingGridEdge, OnConstruction, CalculateEdges, FindNearestEdge, SetEdgeMesh, GetEdgeMeshTransform, SpawnEdgeMeshes, EdgeLines, DrawEdges. 모두 격자·변 데이터와 그 위치·렌더링이라 1인칭 전용 로직 없음.
- FirstPerson: AAsteriaPlayer의 시선 광선·미리보기·ShouldFlip·bPlaceFlipped·메시 선택·Place 입력. 기존대로 Player/에 둠(이동 없음).

## 흐름

```
흐름 1: 플레이어 시작 시 격자 찾기
{Player/AsteriaPlayer.cpp}
  BeginPlay()
    → TActorIterator<ABuildingGrid>             [→ Grid 설정]

흐름 2: 격자 구성 (에디터 배치·속성 변경·레벨 로드)
{Common/BuildingGrid.cpp}
  OnConstruction()
    → CalculateEdges()                          [GridSize 읽음 → Edges 재구성]
    → SpawnEdgeMeshes()                         [Edges 읽음]
    → DrawEdges()                               [Edges 읽음]

흐름 3: 매 프레임 조준 변 찾기·미리보기 (1인칭 전용)
{Player/AsteriaPlayer.cpp}
  Tick()
    → {Common/BuildingGrid.cpp} FindNearestEdge(Ray)   [Edges 읽음]
    → {Player/AsteriaPlayer.cpp} UpdatePreview(Edge, ViewOrigin)
      → ShouldFlip(Edge, ViewOrigin)            [bPlaceFlipped 읽음]

흐름 4: 클릭으로 변에 메시 배치 (1인칭 전용)
{Player/AsteriaPlayer.cpp}
  Place()
    → {Common/BuildingGrid.cpp} FindNearestEdge(Ray)
    → {Player/AsteriaPlayer.cpp} ShouldFlip(Edge, Ray.Origin)
    → {Common/BuildingGrid.cpp} SetEdgeMesh(Vertex, Axis, Mesh, bFlip)   [Edges의 Mesh·bFlip 설정]
      → SpawnEdgeMeshes()
```

## 1단계
- 제약: 기능 변경 금지. 파일 위치와 include 경로만 바꾼다. 클래스·구조체·멤버 이름, 본문, 주석 그대로.
- 제약: 클래스 이름 ABuildingGrid·구조체 이름 FBuildingGridEdge 유지 → /Script/Asteria 경로 불변, CoreRedirects 불필요(BP_BuildingGrid·Buildings.umap 그대로 로드).
- Source/Asteria/Building/Common/BuildingGrid.h (이동: Source/Asteria/Building/BuildingGrid.h를 git mv)
  - 내용 변경 없음 — 재사용: Source/Asteria/Building/BuildingGrid.h
- Source/Asteria/Building/Common/BuildingGrid.cpp (이동·수정: Source/Asteria/Building/BuildingGrid.cpp를 git mv)
  - #include "Building/BuildingGrid.h" → #include "Building/Common/BuildingGrid.h" — 수정
- Source/Asteria/Player/AsteriaPlayer.cpp (수정)
  - #include "Building/BuildingGrid.h" → #include "Building/Common/BuildingGrid.h" — 수정
