요청: 이제 탭 메뉴의 그리드에서, 각 변마다 어떤 메시를 사용할지 지정할 수 있어야 해. 추후에는 사각형 내부에 외곽선이 아닌 변도 메시를 지을 수 있어서, 이거와 방식을 구분짓는게 핵심. (추가 요청) 셀 단위 선택 기능을 지우고 위젯의 그리드에서는 변 단위로만 그린다. 닫히지 않은 변 검증과 닫힌 영역에서 외곽선을 뽑는 로직 추가. 외곽선용·내부 변용 메시를 따로 구분해 두고, 외곽선 변은 외곽선용 메시 중에서, 내부 변은 내부용 메시 중에서 변마다 골라 할당. 한 번 클릭하면 그 위치 변 하나, 드래그하면 시작점에서 끝점까지. 반영은 벽 생성 버튼으로 일괄. 위젯에서 메시별 색으로 표시.
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

## 흐름

```
흐름 1: 탭 메뉴 최초 초기화 — 콤보 두 개 채우고 바인딩
{UI/TabMenu/TabMenuWidget.cpp}
  ~NativeOnInitialized()                     [EdgeMeshData 읽음]
    → +HandleMeshSelectionChanged()          (콤보 채운 뒤 1회 직접 호출)
      → {UI/TabMenu/BuildingGridWidget.cpp} +SetMeshChoices(MeshData, OutlineIndex, InteriorIndex)   [→ MeshData·OutlineMeshIndex·InteriorMeshIndex 설정]
    ⇢ {UI/TabMenu/TabMenuWidget.cpp} OutlineMeshComboBox·InteriorMeshComboBox OnSelectionChanged → +HandleMeshSelectionChanged()
    ⇢ BuildWallsButton OnClicked → ~HandleBuildWallsClicked()

흐름 2: 탭 메뉴를 열 때 격자의 변을 읽어 옴
{UI/TabMenu/BuildingGridWidget.cpp}
  ~NativeConstruct()                         [→ Grid 설정, Grid->Edges 중 Mesh 있는 변 → WallEdges 복사, DragStart·HoveredPos 비움]

흐름 3: 마우스 위치 추적
{UI/TabMenu/BuildingGridWidget.cpp}
  ~NativeOnMouseMove()
    → +GetGridPos(Geometry, ScreenPos)       [→ HoveredPos 설정]
  ~NativeOnMouseLeave()                      [HoveredPos 비움]

흐름 4: 클릭·드래그로 변 그리기(왼쪽)·지우기(오른쪽)
{UI/TabMenu/BuildingGridWidget.cpp}
  ~NativeOnMouseButtonDown()
    → +GetGridPos(Geometry, ScreenPos)       [→ DragStart·bErasing 설정, 마우스 캡처]   (DragStart 없을 때)
                                             [DragStart 비움, 캡처 해제]   (DragStart 있고 다른 버튼일 때: 취소)
  +NativeOnMouseButtonUp()                   [DragStart·bErasing 읽음]
    → +GetGridPos(Geometry, ScreenPos)
    → +GetStrokeEdges(DragStart, 놓은 위치)
      → {Building/Common/BuildingGrid.cpp} +FindNearestEdgeAt(GridPos)   (스냅한 두 꼭짓점이 같을 때: 클릭)
    → {UI/TabMenu/BuildingGridWidget.cpp} WallEdges에서 획의 변 제거   (bErasing)
    → WallEdges에 획의 변 추가   (그리기)
      → {Building/Common/BuildingGrid.cpp} +GetFloorCells(WallEdges, GridSize)
      → +GetEdgeKind(Edge, FloorCells, bOutInwardFlip)   (획의 변마다)
      → {UI/TabMenu/BuildingGridWidget.cpp} +GetMeshEntry(Edge, Kind)   [MeshData·OutlineMeshIndex·InteriorMeshIndex 읽음 → 그 변 Mesh 설정]
    [DragStart 비움, 캡처 해제]

흐름 5: 매 프레임 그리기
{UI/TabMenu/BuildingGridWidget.cpp}
  ~NativePaint()                             [WallEdges·DragStart·bErasing·HoveredPos 읽음]
    → {Building/Common/BuildingGrid.cpp} +GetFloorCells(WallEdges, GridSize)
    → {UI/TabMenu/BuildingGridWidget.cpp} FSlateDrawElement::MakeBox()   (바닥 칸마다, FloorColor)
    → FSlateDrawElement::MakeLines()         (격자 선, LineColor)
    → {Building/Common/BuildingGrid.cpp} +GetEdgeKind()   (WallEdges 변마다)
    → {UI/TabMenu/BuildingGridWidget.cpp} +GetMeshEntry() → FSlateDrawElement::MakeLines()   (엔트리 Color, Open이거나 엔트리 없으면 OpenColor)
    → +GetStrokeEdges(DragStart 또는 HoveredPos, HoveredPos) → FSlateDrawElement::MakeLines()   (HoveredPos 있을 때, PreviewColor·EraseColor)

흐름 6: 벽 생성 버튼으로 격자에 반영
{UI/TabMenu/TabMenuWidget.cpp}
  ~HandleBuildWallsClicked()
    → {UI/TabMenu/BuildingGridWidget.cpp} +BuildWalls()   [WallEdges 읽음]
      → {Building/Common/BuildingGrid.cpp} +GetFloorCells() → +GetEdgeKind()
      → {UI/TabMenu/BuildingGridWidget.cpp} +GetMeshEntry()   (변마다 메시 확정)
      → {Building/Common/BuildingGrid.cpp} +ApplyWallEdges(NewWallEdges)   [Edges 읽음 → FloorCells 설정]
        → +GetFloorCells() → +GetEdgeKind()   (Open이거나 Mesh null인 변이 있으면 false, 변경 없음)
        → SetEdgeMeshes()                    (외곽선 bFlip = 안쪽, 이전에 메시가 있었지만 빠진 변 = null)
      [→ WallEdges 설정]                     (true일 때)

흐름 7: 1인칭 메시 선택·시선 변
{Player/AsteriaPlayer.cpp}
  ~SelectMesh()                              [EdgeMeshData->OutlineMeshes·InteriorMeshes 수 읽음 → SelectedMeshIndex 설정]
  ~GetSelectedMesh()                         (SelectedMeshIndex < OutlineMeshes 수면 OutlineMeshes, 아니면 InteriorMeshes)
  Tick() / Place()
    → {Building/Common/BuildingGrid.cpp} ~FindNearestEdge(Ray)
      → +FindNearestEdgeAt(GridPos)
```

## 1단계
- Source/Asteria/Building/Common/BuildingEdgeMeshData.h (수정)
  - USTRUCT() struct FBuildingEdgeMeshEntry — 메시 하나와 위젯 표시 색. 같은 헤더, UBuildingEdgeMeshData 위에 선언 — 신규
    - TObjectPtr<UStaticMesh> Mesh (UPROPERTY(EditAnywhere, Category = "Placement")) — 변에 놓을 메시
    - FLinearColor Color (UPROPERTY(EditAnywhere, Category = "Placement"), 기본 FLinearColor::White) — UBuildingGridWidget이 이 메시의 변을 칠할 색
  - OutlineMeshes: TArray<FBuildingEdgeMeshEntry> (UPROPERTY(EditAnywhere, Category = "Placement")) — 외곽선 변에 놓을 수 있는 메시. AAsteriaPlayer·UTabMenuWidget·UBuildingGridWidget이 읽음 — 신규
  - InteriorMeshes: TArray<FBuildingEdgeMeshEntry> (UPROPERTY(EditAnywhere, Category = "Placement")) — 내부 변에 놓을 수 있는 메시. 같은 사용처 — 신규
  - 제약: EdgeMeshes는 이 단계에서 지우지 않음(3단계에서 삭제). 클래스 주석의 "1인칭 배치와 탭 메뉴가 같은 에셋" 유지.
- Source/Asteria/Building/Common/BuildingGrid.h (수정)
  - enum class EBuildingEdgeKind : uint8 { Open, Outline, Interior } — 바닥 칸 기준 변 종류. Open = 양쪽 모두 바닥 아님(닫히지 않은 변), Outline = 한쪽만 바닥, Interior = 양쪽 모두 바닥. UENUM 아님. FBuildingGridEdge 위에 선언 — 신규
  - const FBuildingGridEdge* FindNearestEdgeAt(const FVector2D& GridPos) const (public) — 격자 칸 단위 좌표(액터 기준 X/CellSize, Y/CellSize)에서 가장 가까운 변. 후보가 없으면 nullptr. FindNearestEdge()·UBuildingGridWidget::GetStrokeEdges()가 호출 — 신규
  - static TSet<FIntPoint> GetFloorCells(const TArray<FBuildingGridEdge>& WallEdges, const FIntPoint& InGridSize) (public) — WallEdges(Vertex·Axis만 사용)로 둘러싸인 칸 집합. UBuildingGridWidget·ApplyWallEdges()가 호출 — 신규
  - static EBuildingEdgeKind GetEdgeKind(const FBuildingGridEdge& Edge, const TSet<FIntPoint>& InFloorCells, bool& bOutInwardFlip) (public) — Edge 양옆 칸의 바닥 여부로 종류. bOutInwardFlip = 뒤집지 않은 메시의 앞쪽 칸이 바닥이 아니면 true(Outline일 때 안쪽을 향하게 하는 bFlip). UBuildingGridWidget·ApplyWallEdges()가 호출 — 신규
  - bool ApplyWallEdges(const TArray<FBuildingGridEdge>& NewWallEdges) (public) — NewWallEdges를 격자 변 메시로 반영하고 FloorCells를 갱신. 실패 시 false·변경 없음. UBuildingGridWidget::BuildWalls()가 호출 — 신규
  - FloorCells 주석 — "외곽선 벽의 기준. ApplyFloorCells()가 설정"을 "벽 변으로 둘러싸인 칸. ApplyWallEdges()가 설정"으로 — 수정
  - 제약: 매개변수 이름은 멤버(GridSize·FloorCells)를 가리지 않도록 InGridSize·InFloorCells(C4458).
  - 제약: ApplyFloorCells()·GetOutlineEdges()는 이 단계에서 지우지 않음(3단계에서 삭제).
- Source/Asteria/Building/Common/BuildingGrid.cpp (수정)
  - FindNearestEdgeAt() — 기존 FindNearestEdge()의 U·V 이후 로직(FindEdge 람다, RoundU·RoundV, XEdge·YEdge 후보, 거리 비교, ponytail 주석)을 그대로 옮김. U = GridPos.X, V = GridPos.Y — 신규
  - FindNearestEdge() — 평면 교차·Local 계산까지 유지하고 FindNearestEdgeAt(FVector2D(Local.X / CellSize, Local.Y / CellSize)) 결과 반환 — 수정
  - GetFloorCells() — 칸 범위를 격자 밖 한 칸 포함 X = -1..InGridSize.X, Y = -1..InGridSize.Y로 두고 (-1, -1)에서 4방향 BFS. 칸 (X, Y)↔(X+1, Y) 사이 변 = Axis 1, Vertex (X+1, Y). 칸 (X, Y)↔(X, Y+1) 사이 변 = Axis 0, Vertex (X, Y+1). 그 변이 WallEdges에 있으면 건너지 못함. 결과 = 0..InGridSize-1 범위 칸 중 방문하지 못한 칸 — 신규
  - 제약: 벽 조회는 WallEdges로 만든 로컬 TSet<FIntVector>(Vertex.X, Vertex.Y, Axis). ponytail 주석: 벽으로 둘러싸인 빈 공간(안뜰)도 바닥으로 판정, 구분이 필요하면 바닥 칸을 따로 지정.
  - GetEdgeKind() — 기존 GetOutlineEdges()의 칸 규칙 사용. Axis 0: 앞 칸 (X, Y-1), 뒤 칸 (X, Y). Axis 1: 앞 칸 (X, Y), 뒤 칸 (X-1, Y). 바닥인 칸 수 0 = Open, 1 = Outline, 2 = Interior. bOutInwardFlip = !InFloorCells.Contains(앞 칸) — 신규
  - 제약: 앞 칸 근거 주석을 GetOutlineEdges()에서 옮김(뒤집지 않은 메시의 앞은 로컬 -Y, AAsteriaPlayer::ShouldFlip. 세로 변은 yaw 90이라 앞이 +X).
  - ApplyWallEdges() — NewFloor = GetFloorCells(NewWallEdges, GridSize). NewWallEdges 복사본의 각 원소에 GetEdgeKind(원소, NewFloor, bInward): Open이거나 Mesh가 null이면 UE_LOG Warning(변 Vertex·Axis 포함) 후 false 반환(아무것도 바꾸지 않음). Outline이면 원소 bFlip = bInward, Interior면 원소 bFlip 그대로. 그다음 Edges 중 Mesh가 있고 (Vertex, Axis)가 NewWallEdges에 없는 변을 Mesh = null로 복사본에 추가. FloorCells = NewFloor. 복사본이 비어 있지 않으면 SetEdgeMeshes(복사본). true 반환. 재사용: SetEdgeMeshes(같은 파일) — 신규
  - 제약: (Vertex, Axis) 비교는 선형 탐색, ponytail 주석 형식은 SetEdgeMeshes와 동일. 로컬 호출만(서버 권위는 추후).
  - 제약: 검증은 반영 전에 전부 끝냄. 일부만 반영되는 경우 없음.

## 2단계
- Source/Asteria/Player/AsteriaPlayer.h (수정)
  - SelectedMeshIndex 주석 — "EdgeMeshData->EdgeMeshes 인덱스"를 "EdgeMeshData->OutlineMeshes 뒤에 InteriorMeshes를 이어 붙인 순서의 인덱스"로 — 수정
- Source/Asteria/Player/AsteriaPlayer.cpp (수정)
  - SelectMesh() — Num = EdgeMeshData가 있으면 OutlineMeshes.Num() + InteriorMeshes.Num(), 없으면 0. 나머지 그대로 — 수정
  - GetSelectedMesh() — EdgeMeshData null이면 nullptr. SelectedMeshIndex < OutlineMeshes.Num()이면 OutlineMeshes[SelectedMeshIndex].Mesh, 아니면 InteriorMeshes.IsValidIndex(SelectedMeshIndex - OutlineMeshes.Num())일 때 그 원소 Mesh, 아니면 nullptr — 수정
- Source/Asteria/UI/TabMenu/BuildingGridWidget.h (수정)
  - 클래스 주석 — "범위를 선으로 그리는 표시 전용 위젯"을 "격자에 벽 변을 그리고 지우는 편집 위젯"으로 — 수정
  - SelectionColor → FloorColor: FLinearColor (UPROPERTY(EditAnywhere, Category = "Grid"), protected, 기본 FLinearColor(0.2f, 0.6f, 1.f, 0.5f)) — 벽으로 둘러싸인 바닥 칸 색 — 수정
  - OutlineColor — 삭제 — 수정
  - PreviewColor: FLinearColor (UPROPERTY(EditAnywhere, Category = "Grid"), protected, 기본 FLinearColor(1.f, 1.f, 1.f, 0.5f)) — 그리기 획·가리킨 변 미리보기 색 — 신규, 추가
  - EraseColor — 주석 "제거 미리보기 색"을 "지우기 획 미리보기 색"으로, 기본값 그대로 — 수정
  - OpenColor: FLinearColor (UPROPERTY(EditAnywhere, Category = "Grid"), protected, 기본 FLinearColor::Red) — 닫히지 않은 변·메시를 정할 수 없는 변 색 — 신규
  - SelectionAnchor·HoveredCell·bEraseSelection·SelectedCells — 삭제 — 수정
  - MeshData: TObjectPtr<UBuildingEdgeMeshData> (UPROPERTY(Transient), private) — 메시 목록. SetMeshChoices()가 설정 — 신규
  - OutlineMeshIndex: int32 (private, 기본 INDEX_NONE) — 외곽선 콤보에서 고른 MeshData->OutlineMeshes 인덱스 — 신규
  - InteriorMeshIndex: int32 (private, 기본 INDEX_NONE) — 내부 콤보에서 고른 MeshData->InteriorMeshes 인덱스 — 신규
  - WallEdges: TArray<FBuildingGridEdge> (UPROPERTY(Transient), private) — 편집 중인 벽 변(Vertex·Axis·Mesh·bFlip). 열 때 Grid->Edges 중 Mesh 있는 변에서 복사, BuildWalls()로 반영. 반영하지 않은 편집은 다시 열면 버려짐. Mesh null = 반영 때 종류별 콤보 메시 — 신규
  - DragStart: TOptional<FVector2D> (private) — 버튼을 누른 격자 칸 단위 좌표. 없으면 획 진행 중 아님 — 신규
  - bErasing: bool (private, 기본 false) — DragStart가 오른쪽 버튼(지우기)이면 true — 신규
  - HoveredPos: TOptional<FVector2D> (private) — 마우스 아래 격자 칸 단위 좌표. 위젯 밖이면 없음 — 신규
  - void SetMeshChoices(UBuildingEdgeMeshData* InMeshData, int32 InOutlineMeshIndex, int32 InInteriorMeshIndex) (public) — 세 멤버 설정. UTabMenuWidget::HandleMeshSelectionChanged()가 호출 — 신규
  - void BuildWalls() (public) — WallEdges의 메시를 확정해 Grid->ApplyWallEdges()로 반영. UTabMenuWidget::HandleBuildWallsClicked()가 호출 — 신규
  - BuildOutlineWalls(UStaticMesh* Mesh) (public) — 임시: Mesh 무시하고 BuildWalls() 호출. 3단계에서 삭제 — 수정
  - NativeConstruct() 주석 — WallEdges를 Grid->Edges에서 다시 읽고 DragStart·HoveredPos를 비운다로 — 수정
  - NativeOnMouseMove() 주석 — HoveredPos 저장 — 수정
  - NativeOnMouseLeave() 주석 — HoveredPos 비움 — 수정
  - NativeOnMouseButtonDown() 주석 — 왼쪽 = 그리기, 오른쪽 = 지우기 획 시작, 획 중 다른 버튼이면 취소 — 수정
  - virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override (protected) — 획 확정 — 신규
  - NativePaint() 주석 — 바닥 칸, 격자 선, 벽 변(메시 색), 획 미리보기를 그린다 — 수정
  - OverlapsSelectedCells()·GetCellAt() — 삭제 — 수정
  - TOptional<FVector2D> GetGridPos(const FGeometry& Geometry, const FVector2D& ScreenPos) const (private) — 화면 좌표의 격자 칸 단위 좌표(위젯 로컬 / GetCellSize). Grid 무효거나 칸 크기 0이면 빈 값. 격자 밖도 값 반환 — 신규
  - TArray<FBuildingGridEdge> GetStrokeEdges(const FVector2D& Start, const FVector2D& End) const (private) — 획의 변 목록(Vertex·Axis만 채움, Mesh null). NativeOnMouseButtonUp()·NativePaint()가 호출 — 신규
  - const FBuildingEdgeMeshEntry* GetMeshEntry(const FBuildingGridEdge& Edge, EBuildingEdgeKind Kind) const (private) — Edge에 쓸 메시 엔트리. NativeOnMouseButtonUp()·NativePaint()·BuildWalls()가 호출 — 신규
  - 제약: FBuildingEdgeMeshEntry·UBuildingEdgeMeshData 사용을 위해 헤더에 BuildingEdgeMeshData.h 포함 또는 전방 선언(struct FBuildingEdgeMeshEntry; class UBuildingEdgeMeshData;).
- Source/Asteria/UI/TabMenu/BuildingGridWidget.cpp (수정)
  - SetMeshChoices() — MeshData·OutlineMeshIndex·InteriorMeshIndex 설정 — 신규
  - NativeConstruct() — 격자 찾기·SetVisibility 유지. Grid 유효하면 WallEdges = Grid->Edges 중 Mesh != null인 원소 복사, 아니면 Reset. DragStart·HoveredPos Reset — 수정
  - GetGridPos() — Cell = GetCellSize(Geometry.GetLocalSize()), 0 이하면 빈 값. Geometry.AbsoluteToLocal(ScreenPos) / Cell — 신규
  - NativeOnMouseMove() — HoveredPos = GetGridPos(...) — 수정
  - NativeOnMouseLeave() — HoveredPos Reset — 수정
  - NativeOnMouseButtonDown() — 왼쪽·오른쪽 외 버튼은 Super 결과. Pos = GetGridPos, 빈 값이면 FReply::Unhandled(). DragStart가 없으면 DragStart = Pos, bErasing = (오른쪽), FReply::Handled().CaptureMouse(TakeWidget()). DragStart가 있고 버튼이 bErasing과 다르면 DragStart Reset 후 FReply::Handled().ReleaseMouseCapture(). 같은 버튼이면 FReply::Handled() — 수정
  - NativeOnMouseButtonUp() — DragStart가 없거나 버튼이 bErasing과 다르면 Super 결과. Pos = GetGridPos, 빈 값이면 DragStart Reset 후 FReply::Handled().ReleaseMouseCapture(). Stroke = GetStrokeEdges(DragStart, Pos). bErasing이면 Stroke의 (Vertex, Axis)와 같은 원소를 WallEdges에서 제거. 아니면 WallEdges에 없는 키는 추가(Mesh null, bFlip false), Floor = ABuildingGrid::GetFloorCells(WallEdges, Grid->GridSize), Stroke 키에 해당하는 WallEdges 원소마다 Mesh = null로 둔 뒤 Entry = GetMeshEntry(원소, GetEdgeKind(원소, Floor, 무시할 bool)), Mesh = Entry ? Entry->Mesh : null. 끝으로 DragStart Reset, FReply::Handled().ReleaseMouseCapture() — 신규
  - 제약: 이미 있는 변 위에 다시 그리면 그 변의 Mesh만 지금 콤보 메시로 바뀜(덮어쓰기 = 변별 메시 지정). Open 변은 Mesh null.
  - GetStrokeEdges() — Grid 무효면 빈 배열. S·E = Start·End를 반올림해 X는 0..GridSize.X, Y는 0..GridSize.Y로 Clamp한 꼭짓점. S == E면 Grid->FindNearestEdgeAt(End)가 있을 때 그 변의 Vertex·Axis 한 개. 아니면 |E.X - S.X| >= |E.Y - S.Y|일 때 가로: Axis 0, Vertex (X, S.Y), X = Min(S.X, E.X)..Max(S.X, E.X)-1. 그 외 세로: Axis 1, Vertex (S.X, Y), Y = Min(S.Y, E.Y)..Max(S.Y, E.Y)-1 — 신규
  - 제약: 한 번 클릭(누른·놓은 위치가 같은 꼭짓점으로 스냅) = 가장 가까운 변 하나, 드래그 = 시작 꼭짓점에서 끝 꼭짓점까지 우세 축 직선.
  - GetMeshEntry() — MeshData null이거나 Kind == Open이면 nullptr. List = Kind == Outline ? MeshData->OutlineMeshes : MeshData->InteriorMeshes, Index = Kind == Outline ? OutlineMeshIndex : InteriorMeshIndex. Edge.Mesh가 null이 아니고 List에 Mesh가 같은 엔트리가 있으면 그 엔트리, 아니면 List.IsValidIndex(Index)일 때 &List[Index], 아니면 nullptr — 신규
  - 제약: 다른 종류 목록의 메시를 가진 변(그린 뒤 종류가 바뀐 변)과 Mesh null인 변은 그 종류의 콤보 메시로 표시·반영.
  - NativePaint() — Floor = ABuildingGrid::GetFloorCells(WallEdges, Size), 칸마다 기존 DrawRect로 FloorColor(LayerId+1). 격자 선 그대로(LayerId+2). WallEdges 원소마다 Kind = GetEdgeKind, Entry = GetMeshEntry, 색 = Kind != Open && Entry ? Entry->Color : OpenColor, 두께 3 MakeLines(LayerId+3). HoveredPos가 있으면 GetStrokeEdges(DragStart가 있으면 DragStart 아니면 HoveredPos, HoveredPos) 변마다 두께 3 MakeLines, 색 = DragStart 있고 bErasing이면 EraseColor 아니면 PreviewColor(LayerId+4). 변 끝점 계산은 기존 외곽선 코드(Vertex + Axis 방향 한 칸) 그대로. 반환 LayerId+4 — 수정
  - BuildWalls() — Grid 무효면 무시. Floor = ABuildingGrid::GetFloorCells(WallEdges, Grid->GridSize). Resolved = WallEdges 복사, 원소마다 Entry = GetMeshEntry(원소, GetEdgeKind(원소, Floor, 무시할 bool)), Mesh = Entry ? Entry->Mesh : null. Grid->ApplyWallEdges(Resolved)가 true면 WallEdges = Resolved — 신규
  - BuildOutlineWalls() — BuildWalls() 호출만 — 수정
  - OverlapsSelectedCells()·GetCellAt() — 정의 삭제 — 수정
  - 제약: 변 키 (Vertex, Axis) 탐색은 선형 탐색, ponytail 주석 형식은 ABuildingGrid::SetEdgeMeshes와 동일.
  - 제약: 검증(Open 변·메시 null)은 ApplyWallEdges가 담당. 위젯은 OpenColor로 보여 주기만 함.

## 3단계
- Source/Asteria/UI/TabMenu/TabMenuWidget.h (수정)
  - EdgeMeshComboBox → OutlineMeshComboBox: TObjectPtr<UComboBoxString> (UPROPERTY(meta=(BindWidget))) — 외곽선 메시 선택. 옵션 인덱스 = EdgeMeshData->OutlineMeshes 인덱스 — 수정
  - InteriorMeshComboBox: TObjectPtr<UComboBoxString> (UPROPERTY(meta=(BindWidget))) — 내부 메시 선택. 옵션 인덱스 = EdgeMeshData->InteriorMeshes 인덱스 — 신규
  - EdgeMeshData 주석 — "콤보박스 두 개에 채울 메시 목록 에셋"으로 — 수정
  - BuildingGridWidget 주석 — "벽 변을 그리는 격자 위젯"으로 — 수정
  - UFUNCTION() void HandleMeshSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType) — 두 콤보 인덱스를 BuildingGridWidget->SetMeshChoices()로 넘김. OnSelectionChanged(FOnSelectionChangedEvent) 바인딩 대상이라 UFUNCTION 필수 — 신규
  - HandleBuildWallsClicked() 주석 — BuildingGridWidget->BuildWalls() 호출로 — 수정
- Source/Asteria/UI/TabMenu/TabMenuWidget.cpp (수정)
  - NativeOnInitialized() — OutlineMeshComboBox는 OutlineMeshes, InteriorMeshComboBox는 InteriorMeshes의 원소마다 AddOption(GetNameSafe(원소.Mesh)), 목록이 비어 있지 않으면 SetSelectedIndex(0). EdgeMeshData null이면 기존 경고 로그 후 두 콤보 비움. 채운 뒤 HandleMeshSelectionChanged(FString(), ESelectInfo::Direct) 1회 호출, 두 콤보 OnSelectionChanged.AddDynamic(this, &UTabMenuWidget::HandleMeshSelectionChanged), 버튼 바인딩 그대로 — 수정
  - HandleMeshSelectionChanged() — BuildingGridWidget->SetMeshChoices(EdgeMeshData, OutlineMeshComboBox->GetSelectedIndex(), InteriorMeshComboBox->GetSelectedIndex()) — 신규
  - HandleBuildWallsClicked() — 메시 검증 제거, BuildingGridWidget->BuildWalls()만. 주석 "로컬 호출만. 서버 권위는 추후 일괄 적용." 유지 — 수정
  - 제약: 바인딩은 SetSelectedIndex 이후(초기 채우기 중 핸들러 중복 호출 방지).
- Source/Asteria/UI/TabMenu/BuildingGridWidget.h (수정)
  - BuildOutlineWalls(UStaticMesh* Mesh) — 삭제, class UStaticMesh 전방 선언도 다른 사용처 없으면 삭제 — 수정
- Source/Asteria/UI/TabMenu/BuildingGridWidget.cpp (수정)
  - BuildOutlineWalls() — 정의 삭제 — 수정
- Source/Asteria/Building/Common/BuildingEdgeMeshData.h (수정)
  - EdgeMeshes — 삭제 — 수정
- Source/Asteria/Building/Common/BuildingGrid.h (수정)
  - GetOutlineEdges()·ApplyFloorCells() — 선언 삭제 — 수정
- Source/Asteria/Building/Common/BuildingGrid.cpp (수정)
  - GetOutlineEdges()·ApplyFloorCells() — 정의 삭제 — 수정
  - 검증: 삭제 전 Source 전체에서 EdgeMeshes·GetOutlineEdges·ApplyFloorCells·BuildOutlineWalls·EdgeMeshComboBox 참조가 없는지 grep.
