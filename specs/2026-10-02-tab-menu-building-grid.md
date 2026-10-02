요청: 그리드의 범위(건물을 지을 수 있는 범위)를 UTabMenuWidget에 구현해야 해. 지금은 다른거 다 빼고 시각적으로 보여주는 기능만. 3x3 같은 그리드를 위젯에 표시하는게 전부 (추가 요청 1) 최초 생성이 아니라, 탭 메뉴를 열 때마다 현재 정보를 반영해서 표시. (추가 요청 2) 칸마다 위젯을 만들지 말고 NativePaint로 직접 그리는 방식.
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

## 흐름

```
흐름 1: 탭 메뉴를 열 때 격자 찾기
{Player/AsteriaPlayer.cpp}
  ToggleTabMenu()
    → AddToViewport()                        (TabMenu가 뷰포트에 없을 때)
      → {UI/TabMenu/BuildingGridWidget.cpp} +NativeConstruct()   (WBP_TabMenuWidget 안에 배치된 자식 위젯)
        → TActorIterator<ABuildingGrid>      [→ Grid 설정]

흐름 2: 열려 있는 동안 매 프레임 격자 그리기
{UI/TabMenu/BuildingGridWidget.cpp}
  +NativePaint()                             [Grid의 GridSize 읽음, LineColor 읽음]
    → FSlateDrawElement::MakeLines()         (가로선 GridSize.Y+1개, 세로선 GridSize.X+1개)
```

## 1단계
- Source/Asteria/UI/TabMenu/BuildingGridWidget.h (신규)
  - class ASTERIA_API UBuildingGridWidget : public UUserWidget (UCLASS) — 레벨 격자의 범위(GridSize)를 선으로 그리는 표시 전용 위젯. WBP_TabMenuWidget 안에 배치 — 신규
  - LineColor: FLinearColor (UPROPERTY(EditAnywhere, Category = "Grid"), protected, 기본값 FLinearColor::White) — 격자 선 색. WBP에서 지정 — 신규, 추가
  - Grid: TWeakObjectPtr<ABuildingGrid> (private) — 그릴 격자. 레벨의 첫 번째 ABuildingGrid. NativeConstruct()가 설정, NativePaint()가 읽음 — 신규, 추가
  - virtual void NativeConstruct() override (protected) — 열릴 때마다 레벨의 첫 번째 ABuildingGrid를 찾아 Grid에 저장. AddToViewport() 때마다 엔진이 호출 — 신규
  - virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override (protected) — Grid의 현재 GridSize로 격자 선을 그린다. 엔진이 매 프레임 호출 — 신규
  - 제약: ABuildingGrid는 헤더 상단 전방 선언(class ABuildingGrid;). 기존 위젯 헤더(TabMenuWidget.h)처럼 #include "Blueprint/UserWidget.h", "BuildingGridWidget.generated.h". 한 줄 클래스 주석.
- Source/Asteria/UI/TabMenu/BuildingGridWidget.cpp (신규)
  - #include "BuildingGridWidget.h", "EngineUtils.h", "Rendering/DrawElements.h", "Building/Common/BuildingGrid.h" — 추가
  - NativeConstruct() — Super 호출 후 TActorIterator<ABuildingGrid>(GetWorld())로 첫 번째 격자를 Grid에 저장. 재사용: 격자 찾기 방식은 AAsteriaPlayer::BeginPlay()(Source/Asteria/Player/AsteriaPlayer.cpp)와 동일 — 신규
  - 검증: 레벨에 ABuildingGrid가 없으면 Grid를 비우고 UE_LOG(LogTemp, Warning, ...).
  - NativePaint() — Super 결과 LayerId를 받은 뒤, Grid가 유효하면 칸 크기 = min(위젯 로컬 너비 / GridSize.X, 로컬 높이 / GridSize.Y)로 정하고 위젯 왼쪽 위 기준으로 가로선 GridSize.Y+1개, 세로선 GridSize.X+1개를 FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(), 점 배열, ESlateDrawEffect::None, LineColor)로 그리고 LayerId + 1 반환. 재사용: ABuildingGrid::GridSize(Source/Asteria/Building/Common/BuildingGrid.h) — 신규
  - 검증: Grid가 무효면 아무것도 그리지 않고 Super 결과 반환.
  - 제약: 가로 = X(오른쪽), 세로 = Y(아래쪽). 월드 방향과 맞추지 않음. 선 굵기는 MakeLines 기본값.
  - 제약: GridSize는 매 프레임 읽으므로 열려 있는 동안의 변경도 반영됨. 격자 액터 자체의 교체는 다시 열 때 반영.
  - 제약: UBuildingGridWidget을 부모로 한 WBP 생성, WBP_TabMenuWidget(Content/UI)에 크기를 주는 SizeBox 등과 함께 배치하는 것은 에디터 작업. 이 스펙 범위 밖. 위젯 자체 원하는 크기는 0이므로 WBP에서 크기를 줘야 보임.
