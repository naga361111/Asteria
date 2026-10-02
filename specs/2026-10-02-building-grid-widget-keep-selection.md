요청: 탭 키로 열 때 마다, 선택된 영역(외곽선 포함)이 저장되어 있어야 해.
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

## 흐름

```
흐름 1: 탭 메뉴를 열 때 진행 중 선택만 초기화
{Player/AsteriaPlayer.cpp}
  ToggleTabMenu()
    → AddToViewport()                        (TabMenu는 최초 1회 생성 후 재사용 → SelectedRects 유지)
      → {UI/TabMenu/BuildingGridWidget.cpp} ~NativeConstruct()   [→ Grid 설정, SelectionAnchor·HoveredCell 비움, SelectedRects 유지]

흐름 2: 다시 열린 뒤 외곽선 그리기 (변경 없음)
{UI/TabMenu/BuildingGridWidget.cpp}
  NativePaint()
    → GetOutlineEdges()                      [유지된 SelectedRects 읽음]
```

## 1단계
- Source/Asteria/UI/TabMenu/BuildingGridWidget.cpp (수정)
  - NativeConstruct() — SelectedRects.Reset() 줄 삭제. SelectionAnchor·HoveredCell Reset은 유지. 그 위 주석 "탭 메뉴를 열 때마다 선택 초기화."를 "탭 메뉴를 열 때마다 진행 중인 선택만 초기화. 확정 사각형(SelectedRects)은 유지."로 — 수정
  - 제약: 외곽선은 SelectedRects에서 매 프레임 계산(GetOutlineEdges)하므로 별도 저장 불필요.
  - 제약: 유지 범위는 위젯 인스턴스 수명. AAsteriaPlayer::TabMenu가 최초 1회 생성 후 재사용(Source/Asteria/Player/AsteriaPlayer.cpp ToggleTabMenu)하므로 폰이 살아 있는 동안 유지. 레벨 이동·폰 재생성·게임 재시작 시 사라짐 — 처리하지 않음.
- Source/Asteria/UI/TabMenu/BuildingGridWidget.h (수정)
  - NativeConstruct() 주석 "선택 상태를 초기화"를 "진행 중인 선택(SelectionAnchor·HoveredCell)만 초기화"로 — 수정
  - SelectedRects 주석 끝에 "탭 메뉴를 닫았다 열어도 유지." 추가 — 수정
