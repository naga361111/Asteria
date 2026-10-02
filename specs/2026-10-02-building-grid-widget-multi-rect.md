요청: 이제 사각형 여러개를 그릴 수 있도록.
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

## 흐름

```
흐름 1: 탭 메뉴를 열 때 선택 상태 초기화
{UI/TabMenu/BuildingGridWidget.cpp}
  ~NativeConstruct()                         [SelectionAnchor·HoveredCell 비움, SelectedRects 비움]

흐름 2: 클릭 두 번으로 사각형 추가
{UI/TabMenu/BuildingGridWidget.cpp}
  ~NativeOnMouseButtonDown()
    → GetCellAt(Geometry, ScreenPos)         (왼쪽 버튼)
    → SelectionAnchor 설정                   (왼쪽 버튼, SelectionAnchor 없을 때: 첫 모서리)
    → +OverlapsSelectedRects(Rect)           (왼쪽 버튼, SelectionAnchor 있을 때)   [SelectedRects 읽음]
    → SelectedRects에 추가, SelectionAnchor 비움   (겹치지 않을 때)
    → 무시, SelectionAnchor 유지             (겹칠 때)
    → SelectionAnchor 비움                   (오른쪽 버튼: 취소)

흐름 3: 매 프레임 격자·사각형 그리기
{UI/TabMenu/BuildingGridWidget.cpp}
  ~NativePaint()                             [SelectedRects, SelectionAnchor, HoveredCell, SelectionColor 읽음]
    → FSlateDrawElement::MakeBox()           (SelectedRects 각각: 확정 사각형)
    → +OverlapsSelectedRects(Rect)           (SelectionAnchor·HoveredCell 있을 때)
    → FSlateDrawElement::MakeBox()           (미리보기 사각형: 겹치면 빨강, 아니면 SelectionColor 알파 절반 / SelectionAnchor 없으면 가리킨 칸)
    → FSlateDrawElement::MakeLines()         (격자 선, 기존)
```

## 1단계
- Source/Asteria/UI/TabMenu/BuildingGridWidget.h (수정)
  - SelectedRect: TOptional<FIntRect> → SelectedRects: TArray<FIntRect> (private) — 확정된 사각형들(칸 단위, Min 포함·Max 미포함). 서로 겹치지 않음. 확정할 때마다 추가 — 수정
  - bool OverlapsSelectedRects(const FIntRect& Rect) const (private) — Rect가 SelectedRects 중 하나와 칸을 공유하면 true. 변만 맞닿은 경우는 false. NativeOnMouseButtonDown·NativePaint가 호출 — 신규, 추가
  - 제약: 주석의 "SelectedRect"·"새 선택이 덮어씀" 문구를 SelectedRects·추가 동작에 맞게 고침. 나머지 선언은 그대로.
- Source/Asteria/UI/TabMenu/BuildingGridWidget.cpp (수정)
  - NativeConstruct() — SelectedRect.Reset()을 SelectedRects.Reset()으로 — 수정
  - OverlapsSelectedRects() — SelectedRects를 돌며 FIntRect::Intersect(Rect)가 true인 것이 있으면 true. 재사용: FIntRect::Intersect(엔진, 반열린 사각형이라 변만 맞닿으면 false) — 신규
  - NativeOnMouseButtonDown() — 왼쪽 버튼 두 번째 클릭: 기존과 같은 방식으로 SelectionAnchor와 그 칸을 감싸는 FIntRect를 만든 뒤, OverlapsSelectedRects가 true면 아무것도 바꾸지 않고 FReply::Handled() 반환(SelectionAnchor 유지). false면 SelectedRects.Add 후 SelectionAnchor Reset. 첫 클릭·오른쪽 버튼·그 외 분기는 그대로 — 수정
  - NativePaint() — SelectedRect 하나 대신 SelectedRects 각각을 SelectionColor로 DrawRect. 미리보기 사각형은 OverlapsSelectedRects가 true면 FLinearColor(1.f, 0.f, 0.f, PreviewColor.A)로, 아니면 기존 PreviewColor로. 가리킨 칸·격자 선·레이어·반환값은 그대로 — 수정
  - 제약: 사각형 삭제·되돌리기 없음. 탭 메뉴를 다시 열면 전부 초기화(기존과 동일). 맞닿은 사각형 병합은 하지 않음 — 각자 별도 원소.
  - 제약: 배치·ABuildingGrid는 건드리지 않음.
