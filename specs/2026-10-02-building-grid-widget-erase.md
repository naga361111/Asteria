요청: 제거 기능. tabMenu 상에서도 칸을 제거 가능하고, 버튼으로 반영하면 실제로 해당 위치의 벽을 지워야 해. 파생되는 기능으로, 사각형 옆에 새로 붙여서 외곽선이 사라진 경우도 제거 기능이 작동해야 해. (추가 요청 1) 탭 메뉴는 그리드 액터의 데이터를 읽어와 보여주는 것. 위젯 쪽 기억·1인칭 고려 불필요.
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

## 흐름

```
흐름 1: 탭 메뉴를 열 때 격자의 칸을 읽어 옴
{UI/TabMenu/BuildingGridWidget.cpp}
  ~NativeConstruct()                         [→ Grid 설정, Grid의 FloorCells → SelectedCells 복사, SelectionAnchor·HoveredCell 비움]

흐름 2: 클릭 두 번으로 칸 추가(왼쪽) 또는 제거(오른쪽)
{UI/TabMenu/BuildingGridWidget.cpp}
  ~NativeOnMouseButtonDown()
    → GetCellAt(Geometry, ScreenPos)
    → SelectionAnchor 설정, bEraseSelection 설정   (SelectionAnchor 없을 때: 왼쪽 = 추가, 오른쪽 = 제거)
    → SelectionAnchor 비움                   (SelectionAnchor 있고 다른 버튼일 때: 취소)
    → ~OverlapsSelectedCells(Rect)           (SelectionAnchor 있고 왼쪽·추가일 때)   [SelectedCells 읽음]
    → SelectedCells에 Rect 칸 추가, SelectionAnchor 비움   (겹치지 않을 때)
    → SelectedCells에서 Rect 칸 제거, SelectionAnchor 비움   (SelectionAnchor 있고 오른쪽·제거일 때)

흐름 3: 매 프레임 그리기
{UI/TabMenu/BuildingGridWidget.cpp}
  ~NativePaint()                             [SelectedCells, SelectionAnchor, bEraseSelection, HoveredCell, EraseColor 읽음]
    → FSlateDrawElement::MakeBox()           (SelectedCells 칸마다, 미리보기: 제거면 EraseColor)
    → {Building/Common/BuildingGrid.cpp} +ABuildingGrid::GetOutlineEdges(SelectedCells, GridSize)
    → {UI/TabMenu/BuildingGridWidget.cpp} FSlateDrawElement::MakeLines()   (외곽선 변마다)

흐름 4: 버튼으로 칸을 격자에 반영 → 벽 생성·제거
{UI/TabMenu/TabMenuWidget.cpp}
  HandleBuildWallsClicked()
    → {UI/TabMenu/BuildingGridWidget.cpp} ~BuildOutlineWalls(Mesh)
      → {Building/Common/BuildingGrid.cpp} +ApplyFloorCells(SelectedCells, Mesh)   [FloorCells 읽음 → FloorCells 설정]
        → +GetOutlineEdges(FloorCells, GridSize)       (이전 외곽선)
        → +GetOutlineEdges(NewCells, GridSize)         (새 외곽선)
        → SetEdgeMeshes(NewEdges)            (새 외곽선 = Mesh·안쪽, 이전에만 있던 외곽선 = null)
          → SpawnEdgeMeshes()
```

## 1단계
- Source/Asteria/Building/Common/BuildingGrid.h (수정)
  - FloorCells: TSet<FIntPoint> (UPROPERTY(VisibleAnywhere, Category = "Grid"), public) — 건물 바닥 칸 집합. 외곽선 벽의 기준. ApplyFloorCells()가 설정, UBuildingGridWidget이 읽음 — 신규
  - static TArray<FBuildingGridEdge> GetOutlineEdges(const TSet<FIntPoint>& Cells, const FIntPoint& InGridSize) (public) — Cells의 외곽선 변 목록. 원소의 Vertex·Axis·bFlip(앞면이 Cells 안쪽)만 채우고 Mesh는 null. ApplyFloorCells()·UBuildingGridWidget::NativePaint()가 호출 — 신규
  - void ApplyFloorCells(const TSet<FIntPoint>& NewCells, UStaticMesh* WallMesh) (public) — FloorCells를 NewCells로 바꾸고, 새 외곽선 변에 WallMesh를 안쪽 방향으로, 이전 외곽선에만 있던 변에 null을 넣는다. UBuildingGridWidget::BuildOutlineWalls()가 호출 — 신규
  - 제약: 매개변수 이름은 멤버 GridSize를 가리지 않도록 InGridSize.
- Source/Asteria/Building/Common/BuildingGrid.cpp (수정)
  - GetOutlineEdges() — 기존 UBuildingGridWidget::GetOutlineEdges(Source/Asteria/UI/TabMenu/BuildingGridWidget.cpp)의 판정·bFlip 규칙을 그대로 옮김. 칸 선택 여부는 Cells.Contains(칸). 가로 변(Axis 0, Vertex (X, Y), X = 0..InGridSize.X-1, Y = 0..InGridSize.Y)은 칸 (X, Y-1)과 (X, Y), 세로 변(Axis 1, Vertex (X, Y), X = 0..InGridSize.X, Y = 0..InGridSize.Y-1)은 칸 (X-1, Y)와 (X, Y)의 포함 여부가 다를 때 외곽선. bFlip: Axis 0은 !Contains(X, Y-1), Axis 1은 !Contains(X, Y) — 신규
  - 제약: bFlip 근거 주석도 함께 옮김(뒤집지 않은 메시의 앞은 로컬 -Y, AAsteriaPlayer::ShouldFlip).
  - ApplyFloorCells() — WallMesh가 null이면 무시. OldOutline = GetOutlineEdges(FloorCells, GridSize), Edges = GetOutlineEdges(NewCells, GridSize)의 각 원소 Mesh = WallMesh. OldOutline 중 (Vertex, Axis)가 새 결과에 없는 변을 Mesh = null로 같은 배열에 추가. FloorCells = NewCells. 배열이 비어 있지 않으면 SetEdgeMeshes(배열). 재사용: SetEdgeMeshes(같은 파일, Mesh null이면 빈 변) — 신규
  - 제약: (Vertex, Axis) 비교는 선형 탐색. ponytail 주석 형식은 SetEdgeMeshes와 동일.
  - 제약: 이전·새 외곽선 어디에도 없는 변은 넘기지 않음(기존 메시 유지). 로컬 호출만.

## 2단계
- Source/Asteria/UI/TabMenu/BuildingGridWidget.h (수정)
  - EraseColor: FLinearColor (UPROPERTY(EditAnywhere, Category = "Grid"), protected, 기본값 FLinearColor(0.f, 0.f, 0.f, 0.5f)) — 제거 미리보기 색. WBP에서 지정 — 신규, 추가
  - SelectedRects: TArray<FIntRect> → SelectedCells: TSet<FIntPoint> (private) — 편집 중인 칸 집합. 열 때 Grid의 FloorCells에서 복사, 버튼으로 반영 — 수정
  - bEraseSelection: bool (private, 기본 false) — SelectionAnchor가 제거용(오른쪽 클릭)이면 true — 신규
  - OverlapsSelectedRects(const FIntRect& Rect) const → bool OverlapsSelectedCells(const FIntRect& Rect) const — Rect 안 칸 중 SelectedCells에 있는 것이 있으면 true — 수정
  - IsCellSelected(const FIntPoint& Cell) const — 삭제(ABuildingGrid::GetOutlineEdges가 대체) — 수정
  - GetOutlineEdges() const — 삭제(ABuildingGrid::GetOutlineEdges로 이동) — 수정
  - BuildOutlineWalls(UStaticMesh* Mesh) — SelectedCells를 Grid에 반영 — 수정
  - 제약: 주석의 SelectedRects·"오른쪽 클릭으로 취소"·"선택 유지" 문구를 새 동작에 맞게 고침.
- Source/Asteria/UI/TabMenu/BuildingGridWidget.cpp (수정)
  - NativeConstruct() — 격자를 찾은 뒤 Grid가 유효하면 SelectedCells = Grid->FloorCells, 아니면 SelectedCells Reset. SelectionAnchor·HoveredCell Reset 유지. 주석 "확정 사각형(SelectedRects)은 유지"를 "칸은 격자(FloorCells)에서 읽음"으로 — 수정
  - 제약: 반영하지 않은 편집은 탭 메뉴를 닫았다 열면 버려짐(격자 데이터로 다시 읽음).
  - IsCellSelected()·GetOutlineEdges() — 정의 삭제 — 수정
  - OverlapsSelectedCells() — Rect의 모든 칸에 대해 SelectedCells.Contains가 하나라도 true면 true — 수정
  - NativeOnMouseButtonDown() — 왼쪽·오른쪽 버튼 모두 GetCellAt이 빈 값이면 FReply::Unhandled(). SelectionAnchor가 없으면 그 칸을 SelectionAnchor로, bEraseSelection = (오른쪽 버튼). SelectionAnchor가 있고 버튼이 bEraseSelection과 다르면 SelectionAnchor Reset(취소). 같으면 기존 방식으로 Rect를 만들어: 추가면 OverlapsSelectedCells가 true일 때 아무것도 바꾸지 않음(SelectionAnchor 유지), false면 Rect 칸 전부 SelectedCells.Add 후 SelectionAnchor Reset. 제거면 Rect 칸 전부 SelectedCells.Remove 후 SelectionAnchor Reset. 두 버튼 모두 FReply::Handled(). 그 외 버튼은 Super 결과 — 수정
  - NativePaint() — 확정 박스를 SelectedCells 칸마다 FIntRect(칸, 칸 + (1, 1))로 SelectionColor. 미리보기 사각형: bEraseSelection이면 EraseColor, 아니면 기존 규칙(OverlapsSelectedCells면 빨강, 아니면 PreviewColor). 외곽선은 ABuildingGrid::GetOutlineEdges(SelectedCells, Size) 결과로 기존과 같은 MakeLines. 가리킨 칸·격자 선·레이어·반환값 그대로 — 수정
  - BuildOutlineWalls() — Grid 무효거나 Mesh null이면 무시. Grid->ApplyFloorCells(SelectedCells, Mesh). 기존 외곽선 계산·SetEdgeMeshes 직접 호출 제거 — 수정
  - 제약: UTabMenuWidget은 수정하지 않음(HandleBuildWallsClicked → BuildOutlineWalls 그대로).
