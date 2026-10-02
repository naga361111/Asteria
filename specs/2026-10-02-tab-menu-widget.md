요청: 탭 키를 눌렀을 때, 화면에 특정 UI를 표시. 해당 UI는 UW을 상속해서 구현. (답변: 빈 위젯, 토글, UI가 떠 있는 동안 커서 표시)
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

## 흐름

```
흐름 1: 입력 바인딩
{Player/AsteriaPlayer.cpp}
  ~SetupPlayerInputComponent()
    ⇢ TabMenuAction Started → +ToggleTabMenu()

흐름 2: 탭 키로 메뉴 토글
{Player/AsteriaPlayer.cpp}
  +ToggleTabMenu()                          [TabMenuClass 읽음, TabMenu 읽음/설정]
    → {UI/TabMenu/TabMenuWidget.h} +UTabMenuWidget 생성 (CreateWidget, TabMenu가 null일 때 1회)
    → {Player/AsteriaPlayer.cpp} AddToViewport()      (TabMenu가 뷰포트에 없을 때)
    → SetUIInputMode(true)                  [bUIInputMode 설정]
    → RemoveFromParent()                    (TabMenu가 뷰포트에 있을 때)
    → SetUIInputMode(false)                 [bUIInputMode 설정]
```

## 1단계
- Source/Asteria/UI/TabMenu/TabMenuWidget.h (신규)
  - class ASTERIA_API UTabMenuWidget : public UUserWidget (UCLASS) — 탭 키로 여닫는 화면 UI의 C++ 베이스. 멤버 없음, 내용은 WBP에서 구성. AAsteriaPlayer::TabMenuClass·TabMenu가 참조 — 신규
  - 제약: 기존 위젯 헤더(GameClockWidget.h)처럼 #include "Blueprint/UserWidget.h", "TabMenuWidget.generated.h". 한 줄 클래스 주석.
- Source/Asteria/UI/TabMenu/TabMenuWidget.cpp (신규)
  - #include "TabMenuWidget.h"만 — 신규, 추가
  - 제약: 다른 위젯 .cpp처럼 파일 존재. 본문 없음.

## 2단계
- Source/Asteria/Player/AsteriaPlayer.h (수정)
  - TabMenuAction: TObjectPtr<UInputAction> (UPROPERTY(EditDefaultsOnly, Category = "Input")) — 탭 메뉴 토글 입력 액션. BP_Player 기본값에서 지정 — 신규, 추가
  - TabMenuClass: TSubclassOf<UTabMenuWidget> (UPROPERTY(EditDefaultsOnly, Category = "UI")) — 띄울 WBP 클래스. BP_Player 기본값에서 지정 — 신규
  - TabMenu: TObjectPtr<UTabMenuWidget> (UPROPERTY(Transient)) — 생성된 위젯. 최초 토글 때 1회 생성 후 재사용. UPROPERTY는 GC 보호용 — 신규, 추가
  - void ToggleTabMenu(const FInputActionValue& Value) — 탭 메뉴를 열거나 닫고 커서 모드를 맞춘다. SetupPlayerInputComponent()가 바인딩 — 신규
  - 제약: UTabMenuWidget은 헤더 상단 전방 선언(class UTabMenuWidget;). TSubclassOf는 전방 선언으로 충분.
- Source/Asteria/Player/AsteriaPlayer.cpp (수정)
  - #include "UI/TabMenu/TabMenuWidget.h" — 추가
  - SetupPlayerInputComponent() — TabMenuAction을 ETriggerEvent::Started로 ToggleTabMenu()에 바인딩 — 수정
  - ToggleTabMenu() — TabMenu가 null이면 CreateWidget<UTabMenuWidget>(PC, TabMenuClass)로 생성해 저장. TabMenu->IsInViewport()이면 RemoveFromParent() 후 SetUIInputMode(false), 아니면 AddToViewport() 후 SetUIInputMode(true). 재사용: SetUIInputMode(Source/Asteria/Player/AsteriaPlayer.cpp) — 신규
  - 검증: !IsLocallyControlled()이면 무시. GetController<APlayerController>()가 null이면 무시.
  - 검증: TabMenuClass가 비어 있으면 UE_LOG(LogTemp, Warning, ...) 후 무시 (AAsteriaHUD::CreateMainHUD와 같은 방식).
  - 검증: CreateWidget 결과가 null이면 무시.
  - 제약: 커서 상태(bUIInputMode)는 상호작용(QuestBoard·창구)과 공유. 탭 메뉴가 열린 채 상호작용 범위를 벗어나면 OnDetectionEndOverlap의 SetUIInputMode(false)로 커서가 꺼질 수 있음 — 처리하지 않음.
  - 제약: IA_TabMenu 생성, IMC_Player에 Tab 매핑, UTabMenuWidget을 부모로 한 WBP 생성, BP_Player의 TabMenuAction·TabMenuClass 지정은 에디터 작업. 이 스펙 범위 밖.
