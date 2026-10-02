요청: 이제 프로토타입처럼, 그리드에서 영역을 선택 가능해야 해. 배치는 아직. 지금은 위젯 상에서 선택만 구현.
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

## 흐름

```
흐름 1: 탭 메뉴를 열 때 선택 상태 초기화
{UI/TabMenu/BuildingGridWidget.cpp}
  ~NativeConstruct()                         [→ Grid 설정, SelectionAnchor·HoveredCell·SelectedRect 비움, Visibility = Visible]

흐름 2: 마우스 이동으로 가리킨 칸 갱신
{UI/TabMenu/BuildingGridWidget.cpp}
  +NativeOnMouseMove()
    → +GetCellAt(Geometry, ScreenPos)        [Grid의 GridSize 읽음]
      → +GetCellSize(LocalSize)
    → HoveredCell 설정
  +NativeOnMouseLeave()                      [HoveredCell 비움]

흐름 3: 클릭 두 번으로 영역 선택
{UI/TabMenu/BuildingGridWidget.cpp}
  +NativeOnMouseButtonDown()
    → +GetCellAt(Geometry, ScreenPos)        (왼쪽 버튼)
    → SelectionAnchor 설정                   (왼쪽 버튼, SelectionAnchor 없을 때: 첫 모서리)
    → SelectedRect 설정, SelectionAnchor 비움  (왼쪽 버튼, SelectionAnchor 있을 때: 반대 모서리로 확정)
    → SelectionAnchor 비움                   (오른쪽 버튼: 취소)

흐름 4: 매 프레임 격자·선택 그리기
{UI/TabMenu/BuildingGridWidget.cpp}
  ~NativePaint()                             [Grid의 GridSize, LineColor, SelectionColor, SelectedRect, SelectionAnchor, HoveredCell 읽음]
    → +GetCellSize(LocalSize)
    → FSlateDrawElement::MakeBox()           (SelectedRect 있을 때: 확정 영역)
    → FSlateDrawElement::MakeBox()           (SelectionAnchor·HoveredCell 있을 때: 미리보기 사각형 / SelectionAnchor 없고 HoveredCell 있을 때: 가리킨 칸)
    → FSlateDrawElement::MakeLines()         (격자 선, 기존)
```

## 1단계
- Source/Asteria/UI/TabMenu/BuildingGridWidget.h (수정)
  - SelectionColor: FLinearColor (UPROPERTY(EditAnywhere, Category = "Grid"), protected, 기본값 FLinearColor(0.2f, 0.6f, 1.f, 0.5f)) — 확정 영역 색. 미리보기·가리킨 칸은 같은 색에 알파 절반. WBP에서 지정 — 신규, 추가
  - SelectionAnchor: TOptional<FIntPoint> (private) — 첫 클릭으로 정한 모서리 칸. 없으면 선택 중 아님 — 신규
  - HoveredCell: TOptional<FIntPoint> (private) — 마우스 아래 칸. 격자 밖이면 없음 — 신규
  - SelectedRect: TOptional<FIntRect> (private) — 확정된 선택 영역(칸 단위, Min 포함·Max 미포함). 새 선택이 덮어씀 — 신규
  - virtual void NativeConstruct() override — 기존 동작 + 선택 상태 초기화·Visibility 설정 — 수정
  - virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override (protected) — HoveredCell 갱신 — 신규
  - virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override (protected) — HoveredCell 비움 — 신규, 추가
  - virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override (protected) — 왼쪽 클릭 두 번으로 SelectedRect 확정, 오른쪽 클릭으로 취소 — 신규
  - virtual int32 NativePaint(...) const override — 기존 시그니처 그대로. 선택 표시 추가 — 수정
  - TOptional<FIntPoint> GetCellAt(const FGeometry& Geometry, const FVector2D& ScreenPos) const (private) — 화면 좌표의 칸. 격자 밖·Grid 무효면 빈 값. NativeOnMouseMove·NativeOnMouseButtonDown이 호출 — 신규
  - float GetCellSize(const FVector2D& LocalSize) const (private) — 칸 한 변 길이 = min(LocalSize.X / GridSize.X, LocalSize.Y / GridSize.Y). Grid 무효면 0. GetCellAt·NativePaint가 호출 — 신규, 추가
- Source/Asteria/UI/TabMenu/BuildingGridWidget.cpp (수정)
  - #include "Styling/CoreStyle.h" — 추가
  - NativeConstruct() — 기존 격자 찾기 뒤에 SelectionAnchor·HoveredCell·SelectedRect Reset, SetVisibility(ESlateVisibility::Visible) — 수정
  - 제약: UUserWidget 기본 Visibility는 SelfHitTestInvisible이라 마우스 이벤트를 못 받음. 그래서 Visible로 설정.
  - GetCellSize() — 위 정의대로 — 신규
  - GetCellAt() — Geometry.AbsoluteToLocal(ScreenPos)를 GetCellSize(Geometry.GetLocalSize())로 나눠 FloorToInt32. 0 ≤ X < GridSize.X, 0 ≤ Y < GridSize.Y일 때만 값 반환 — 신규
  - 검증: GetCellSize가 0 이하면 빈 값(0으로 나누기 방지).
  - NativeOnMouseMove() — HoveredCell = GetCellAt(InGeometry, InMouseEvent.GetScreenSpacePosition()). Super 결과 반환 — 신규
  - NativeOnMouseLeave() — Super 호출 후 HoveredCell Reset — 신규
  - NativeOnMouseButtonDown() — 왼쪽 버튼: GetCellAt이 빈 값이면 FReply::Unhandled(). SelectionAnchor가 없으면 그 칸을 SelectionAnchor로, 있으면 SelectionAnchor와 그 칸을 감싸는 FIntRect(Min = 성분별 최소, Max = 성분별 최대 + 1)를 SelectedRect로 저장하고 SelectionAnchor Reset. FReply::Handled(). 오른쪽 버튼: SelectionAnchor Reset 후 FReply::Handled(). 그 외: Super 결과 반환 — 신규
  - 제약: 입력 모드가 GameAndUI(AAsteriaPlayer::SetUIInputMode)라 Handled를 반환해야 게임 입력(PlaceAction 등)으로 클릭이 넘어가지 않음.
  - NativePaint() — 기존 Super 호출·Grid 무효 시 반환 유지. 칸 크기는 GetCellSize(AllottedGeometry.GetLocalSize())로 교체. 격자 선보다 먼저(LayerId + 1) 박스를 그린다: SelectedRect가 있으면 그 영역을 SelectionColor로, SelectionAnchor·HoveredCell이 둘 다 있으면 둘을 감싸는 사각형을 SelectionColor(알파 절반)로, SelectionAnchor 없이 HoveredCell만 있으면 그 칸을 SelectionColor(알파 절반)로. 박스는 FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(크기, FSlateLayoutTransform(왼쪽 위 좌표)), FCoreStyle::Get().GetBrush("WhiteBrush"), ESlateDrawEffect::None, 색). 격자 선은 LayerId + 2로 올리고 LayerId + 2 반환 — 수정
  - 제약: 선택은 화면 표시만. ABuildingGrid·Edges·액터 스폰은 건드리지 않음. 확정 영역은 하나만 유지.
  - 제약: 탭 메뉴를 닫았다 열면 선택 상태 초기화. Esc 취소는 넣지 않음(키보드 포커스 필요).
