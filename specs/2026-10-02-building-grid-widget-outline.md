요청: 이제 그리드에 그린 사각형들에서 외곽선만 추출해서 해당 변들을 강조 해야 해. 그리드에 그려진 모든 사각형에 대해서. 다수의 사각형이 겹칠 경우 겹치는 부분은 외곽선이 아니야 (추가 요청 1) 겹침 차단은 그대로 두고, 맞닿은 경계만 외곽선에서 뺀다.
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

## 흐름

```
흐름 1: 매 프레임 격자·사각형·외곽선 그리기
{UI/TabMenu/BuildingGridWidget.cpp}
  ~NativePaint()                             [SelectedRects, SelectionAnchor, HoveredCell, SelectionColor, LineColor, OutlineColor 읽음]
    → FSlateDrawElement::MakeBox()           (SelectedRects 각각, 미리보기 사각형 / 가리킨 칸: 기존)
    → FSlateDrawElement::MakeLines()         (격자 선, 기존)
    → +IsCellSelected(Cell)                  (변마다 양쪽 칸 두 번)   [SelectedRects 읽음]
    → FSlateDrawElement::MakeLines()         (양쪽 칸 중 하나만 선택된 변: 외곽선)
```

## 1단계
- Source/Asteria/UI/TabMenu/BuildingGridWidget.h (수정)
  - OutlineColor: FLinearColor (UPROPERTY(EditAnywhere, Category = "Grid"), protected, 기본값 FLinearColor(1.f, 0.8f, 0.2f, 1.f)) — 외곽선 색. WBP에서 지정 — 신규, 추가
  - bool IsCellSelected(const FIntPoint& Cell) const (private) — Cell이 SelectedRects 중 하나라도 포함되면 true. NativePaint가 호출 — 신규
  - 제약: NativePaint 주석에 외곽선 추가. 나머지 선언·주석 그대로(OverlapsSelectedRects 유지, 겹침 차단 유지).
- Source/Asteria/UI/TabMenu/BuildingGridWidget.cpp (수정)
  - IsCellSelected() — SelectedRects를 돌며 FIntRect::Contains(Cell)이 true인 것이 있으면 true. 재사용: FIntRect::Contains(엔진, Min 포함·Max 미포함) — 신규
  - NativePaint() — 기존 박스·격자 선은 그대로. 격자 선 뒤에 외곽선을 LayerId + 3으로 그림: 가로 변(Vertex (X, Y), X = 0..GridSize.X-1, Y = 0..GridSize.Y)은 칸 (X, Y-1)과 (X, Y), 세로 변(Vertex (X, Y), X = 0..GridSize.X, Y = 0..GridSize.Y-1)은 칸 (X-1, Y)와 (X, Y)에 대해 IsCellSelected 결과가 서로 다를 때만 외곽선. 선은 FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 3, AllottedGeometry.ToPaintGeometry(), 점 배열, ESlateDrawEffect::None, OutlineColor, true, 3.f). LayerId + 3 반환 — 수정
  - 제약: 사각형끼리는 겹치지 않으므로(기존 OverlapsSelectedRects 차단) 외곽선에서 빠지는 것은 맞닿은 경계. 두 칸이 모두 선택되면 외곽선 아님.
  - 제약: 변 정의는 ABuildingGrid와 같음 — 가로 변 = Axis 0, 세로 변 = Axis 1, Vertex = 변의 시작 꼭짓점(Source/Asteria/Building/Common/BuildingGrid.cpp CalculateEdges()). 격자 밖 칸은 선택되지 않은 칸으로 취급(IsCellSelected가 자연히 false).
  - 제약: 외곽선은 확정 사각형(SelectedRects)만 대상. 미리보기 사각형은 포함하지 않음. 매 프레임 계산(저장하지 않음).
  - 제약: NativeOnMouseButtonDown·배치·ABuildingGrid는 건드리지 않음.
