요청: 1. 데이터 에셋을 Common으로 2. 플레이어 (수정) 3. 격자 위젯
(합의 내용) 외곽선 위치의 실제 월드 격자(ABuildingGrid)에 벽 생성. 생성은 TabMenu의 "벽 생성" 버튼. 메시는 TabMenu 콤보박스로 선택, 지금은 한 종류로 외곽선 전체를 채우지만 추후 변마다 다른 메시를 섞을 계획. 메시 목록은 Common 데이터 에셋 UBuildingEdgeMeshData로 관리하고 플레이어 PlaceableMeshes를 대체. 로컬만(서버 권위는 추후 일괄). 벽 앞면은 선택 영역 안쪽. 외곽선이 아닌 변의 기존 메시는 건드리지 않음.
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

## 흐름

```
흐름 1: 탭 메뉴 최초 생성 시 메시 목록·버튼 준비
{UI/TabMenu/TabMenuWidget.cpp}
  +NativeOnInitialized()                     [EdgeMeshData 읽음 → EdgeMeshComboBox 채움]
    ⇢ BuildWallsButton OnClicked → +HandleBuildWallsClicked()

흐름 2: 벽 생성 버튼으로 외곽선에 벽 배치
{UI/TabMenu/TabMenuWidget.cpp}
  +HandleBuildWallsClicked()                 [EdgeMeshComboBox 선택 인덱스, EdgeMeshData 읽음]
    → {UI/TabMenu/BuildingGridWidget.cpp} +BuildOutlineWalls(Mesh)
      → +GetOutlineEdges()                   [Grid의 GridSize, SelectedRects 읽음 → Vertex·Axis·bFlip(안쪽)]
        → IsCellSelected(Cell)
      → {Building/Common/BuildingGrid.cpp} +SetEdgeMeshes(NewEdges)   [Edges의 Mesh·bFlip 설정]
        → SpawnEdgeMeshes()

흐름 3: 매 프레임 외곽선 그리기
{UI/TabMenu/BuildingGridWidget.cpp}
  ~NativePaint()
    → +GetOutlineEdges()
    → FSlateDrawElement::MakeLines()         (외곽선 변마다)

흐름 4: 1인칭 메시 선택·미리보기·배치
{Player/AsteriaPlayer.cpp}
  ~SelectMesh()                              [EdgeMeshData 읽음 → SelectedMeshIndex 설정]
  Tick()
    → ~UpdatePreview(Edge, ViewOrigin)
      → +GetSelectedMesh()                   [EdgeMeshData, SelectedMeshIndex 읽음]
  ~Place()
    → +GetSelectedMesh()
    → {Common/BuildingGrid.cpp} FindNearestEdge(Ray)
    → {Player/AsteriaPlayer.cpp} ShouldFlip(Edge, Ray.Origin)
    → {Common/BuildingGrid.cpp} +SetEdgeMeshes(NewEdges)   (원소 1개, 기존 ~SetEdgeMesh 대체)
```

## 1단계
- Source/Asteria/Building/Common/BuildingEdgeMeshData.h (신규)
  - class ASTERIA_API UBuildingEdgeMeshData : public UDataAsset (UCLASS) — 격자 변에 놓을 수 있는 메시 목록. 1인칭 배치와 탭 메뉴가 같은 에셋을 씀 — 신규
  - EdgeMeshes: TArray<TObjectPtr<UStaticMesh>> (UPROPERTY(EditAnywhere, Category = "Placement")) — 변에 놓을 메시들. AAsteriaPlayer·UTabMenuWidget이 읽음 — 신규
  - 제약: 기존 데이터 에셋(Source/Asteria/Data/NpcSpawnData.h)처럼 #include "Engine/DataAsset.h", "BuildingEdgeMeshData.generated.h". UStaticMesh는 전방 선언. 한 줄 클래스 주석. 생성자 없음.
- Source/Asteria/Building/Common/BuildingEdgeMeshData.cpp (신규)
  - #include "Building/Common/BuildingEdgeMeshData.h"만 — 신규, 추가
  - 제약: 다른 데이터 에셋 .cpp처럼 파일 존재. 본문 없음.
- Source/Asteria/Building/Common/BuildingGrid.h (수정)
  - void SetEdgeMeshes(const TArray<FBuildingGridEdge>& NewEdges) (public) — NewEdges 각 원소의 (Vertex, Axis) 키 변에 그 원소의 Mesh·bFlip을 넣고, 인스턴스 메시를 한 번만 다시 만든다. 원소의 Transform은 무시. 키에 해당하는 변이 없는 원소는 무시. UBuildingGridWidget::BuildOutlineWalls·AAsteriaPlayer::Place가 호출 — 신규
  - 제약: 기존 SetEdgeMesh는 이 단계에서 유지(3단계에서 삭제).
- Source/Asteria/Building/Common/BuildingGrid.cpp (수정)
  - SetEdgeMeshes() — 원소마다 SetEdgeMesh와 같은 선형 탐색으로 변을 찾아 Mesh·bFlip 설정. 전부 끝난 뒤 SetEdgeMesh와 같은 방식으로 UInstancedStaticMeshComponent 전부 DestroyComponent 후 SpawnEdgeMeshes() 1회. 재사용: SpawnEdgeMeshes(같은 파일) — 신규
  - 제약: ponytail 주석 형식은 기존 SetEdgeMesh와 동일(변마다 선형 탐색, 수백 개 넘으면 TMap).

## 2단계
- Source/Asteria/Player/AsteriaPlayer.h (수정)
  - PlaceableMeshes: TArray<TObjectPtr<UStaticMesh>> — 삭제 — 수정
  - EdgeMeshData: TObjectPtr<UBuildingEdgeMeshData> (UPROPERTY(EditDefaultsOnly, Category = "Placement")) — 잡을 수 있는 메시 목록 에셋. BP_Player 기본값에서 지정 — 신규
  - SelectedMeshIndex: int32 — 주석의 "PlaceableMeshes"를 "EdgeMeshData->EdgeMeshes"로 — 수정
  - UStaticMesh* GetSelectedMesh() const (protected) — EdgeMeshData가 있고 SelectedMeshIndex가 EdgeMeshes의 유효 인덱스면 그 메시, 아니면 nullptr. UpdatePreview·Place가 호출 — 신규, 추가
  - 제약: UBuildingEdgeMeshData는 헤더 상단 전방 선언.
- Source/Asteria/Player/AsteriaPlayer.cpp (수정)
  - #include "Building/Common/BuildingEdgeMeshData.h" — 추가
  - SelectMesh() — Num을 EdgeMeshData ? EdgeMeshData->EdgeMeshes.Num() : 0으로. 나머지 그대로 — 수정
  - GetSelectedMesh() — 위 정의대로 — 신규
  - UpdatePreview() — PlaceableMeshes.IsValidIndex(SelectedMeshIndex) 검사와 PlaceableMeshes[SelectedMeshIndex]를 GetSelectedMesh()(nullptr이면 숨김)로 교체. 나머지 그대로 — 수정
  - Place() — PlaceableMeshes 검사를 GetSelectedMesh() nullptr 검사로 교체. Grid->SetEdgeMesh(...) 대신 *Edge를 복사해 Mesh = 선택 메시, bFlip = ShouldFlip(*Edge, Ray.Origin)으로 바꾼 원소 1개 배열을 Grid->SetEdgeMeshes()에 넘김 — 수정
  - 제약: 기능 변화 없음. EdgeMeshData가 비어 있거나 null이면 기존에 PlaceableMeshes가 비었을 때와 같이 미리보기 숨김·배치 무시.
- Source/Asteria/UI/TabMenu/BuildingGridWidget.h (수정)
  - void BuildOutlineWalls(UStaticMesh* Mesh) (public) — 현재 외곽선 변 전체에 Mesh를 안쪽 방향으로 넣어 Grid->SetEdgeMeshes()로 보낸다. UTabMenuWidget::HandleBuildWallsClicked가 호출 — 신규
  - TArray<FBuildingGridEdge> GetOutlineEdges() const (private) — 외곽선 변 목록. 원소의 Vertex·Axis·bFlip만 채우고 Mesh는 null. NativePaint·BuildOutlineWalls가 호출 — 신규
  - 제약: FBuildingGridEdge를 값으로 반환하므로 #include "Building/Common/BuildingGrid.h"를 헤더에 추가(전방 선언 class ABuildingGrid; 제거 가능). UStaticMesh 전방 선언.
- Source/Asteria/UI/TabMenu/BuildingGridWidget.cpp (수정)
  - GetOutlineEdges() — Grid 무효면 빈 배열. 기존 NativePaint의 외곽선 판정 그대로: 가로 변(Axis 0, Vertex (X, Y), X = 0..GridSize.X-1, Y = 0..GridSize.Y)은 칸 (X, Y-1)과 (X, Y), 세로 변(Axis 1, Vertex (X, Y), X = 0..GridSize.X, Y = 0..GridSize.Y-1)은 칸 (X-1, Y)와 (X, Y)의 IsCellSelected가 다를 때 외곽선. bFlip: Axis 0은 !IsCellSelected(X, Y-1), Axis 1은 !IsCellSelected(X, Y) — 신규
  - 제약: bFlip 근거 — 뒤집지 않은 메시의 앞은 로컬 -Y(AAsteriaPlayer::ShouldFlip, Source/Asteria/Player/AsteriaPlayer.cpp). Axis 0(yaw 0)의 앞은 -Y로 칸 (X, Y-1) 쪽, Axis 1(yaw 90)의 앞은 +X로 칸 (X, Y) 쪽. 그 칸이 선택 영역 밖이면 뒤집어 안쪽을 향하게 함.
  - BuildOutlineWalls() — Grid 무효거나 Mesh null이면 무시. GetOutlineEdges() 결과 각 원소의 Mesh = Mesh로 채워 Grid->SetEdgeMeshes()에 넘김. 결과가 비어 있으면 무시 — 신규
  - NativePaint() — 외곽선 두 이중 루프를 GetOutlineEdges() 결과 순회로 교체. 변마다 Vertex에서 Vertex + (Axis 0이면 (1, 0), 1이면 (0, 1))까지 칸 크기를 곱한 선을 기존과 같은 MakeLines 인자(LayerId + 3, OutlineColor, true, 3.f)로. 나머지 그대로 — 수정
  - 제약: 외곽선이 아닌 변은 SetEdgeMeshes에 넘기지 않으므로 기존 메시 유지. 로컬 호출만(서버 RPC 없음).

## 3단계
- Source/Asteria/UI/TabMenu/TabMenuWidget.h (수정)
  - EdgeMeshData: TObjectPtr<UBuildingEdgeMeshData> (UPROPERTY(EditAnywhere, Category = "Placement"), protected) — 콤보박스에 채울 메시 목록 에셋. WBP_TabMenuWidget에서 지정 — 신규
  - EdgeMeshComboBox: TObjectPtr<UComboBoxString> (UPROPERTY(meta=(BindWidget)), protected) — 외곽선에 넣을 메시 선택 — 신규
  - BuildWallsButton: TObjectPtr<UButton> (UPROPERTY(meta=(BindWidget)), protected) — 벽 생성 버튼 — 신규
  - BuildingGridWidget: TObjectPtr<UBuildingGridWidget> (UPROPERTY(meta=(BindWidget)), protected) — 외곽선을 가진 격자 위젯. WBP_TabMenuWidget에 배치된 WBP_BuildingGrid 인스턴스 — 신규, 추가
  - virtual void NativeOnInitialized() override (protected) — 콤보박스를 채우고 버튼을 바인딩. 엔진이 1회 호출 — 신규
  - void HandleBuildWallsClicked() (UFUNCTION(), protected) — 선택 메시로 BuildingGridWidget->BuildOutlineWalls 호출. BuildWallsButton OnClicked 바인딩 대상 — 신규
  - 제약: UBuildingEdgeMeshData·UComboBoxString·UButton·UBuildingGridWidget 전방 선언. 클래스 주석의 "내용은 WBP에서 구성한다"는 유지.
- Source/Asteria/UI/TabMenu/TabMenuWidget.cpp (수정)
  - #include "Components/ComboBoxString.h", "Components/Button.h", "Building/Common/BuildingEdgeMeshData.h", "UI/TabMenu/BuildingGridWidget.h" — 추가
  - NativeOnInitialized() — Super 호출. EdgeMeshComboBox->ClearOptions() 후 EdgeMeshData->EdgeMeshes 각각 GetNameSafe(메시)를 AddOption, 하나 이상이면 SetSelectedIndex(0). BuildWallsButton->OnClicked.AddDynamic(this, &UTabMenuWidget::HandleBuildWallsClicked). 재사용: 버튼 바인딩 방식은 USettleTileEntryWidget::NativeOnInitialized(Source/Asteria/UI/Counter/WaitForSettle/SettleTileEntryWidget.cpp)와 동일 — 신규
  - 검증: EdgeMeshData가 null이면 UE_LOG(LogTemp, Warning, ...) 후 콤보박스는 비운 채로 둠(버튼 바인딩은 수행).
  - HandleBuildWallsClicked() — EdgeMeshData가 null이거나 EdgeMeshComboBox->GetSelectedIndex()가 EdgeMeshes의 유효 인덱스가 아니면 무시. 그 메시가 null이면 무시. 아니면 BuildingGridWidget->BuildOutlineWalls(메시) — 신규
  - 제약: 콤보박스 인덱스 = EdgeMeshes 인덱스(같은 순서로 채움).
- Source/Asteria/Building/Common/BuildingGrid.h (수정)
  - void SetEdgeMesh(const FIntPoint& Vertex, int32 Axis, UStaticMesh* Mesh, bool bFlip) — 삭제(2단계 후 호출처 없음, SetEdgeMeshes로 대체) — 수정
- Source/Asteria/Building/Common/BuildingGrid.cpp (수정)
  - SetEdgeMesh() — 정의 삭제 — 수정
  - 제약: 삭제 전 Grep으로 SetEdgeMesh( 호출처가 없음을 확인. SetEdgeMeshes는 그대로.
- 제약(에디터 작업, 범위 밖): DA_BuildingEdgeMeshData(Content/Data) 생성 후 기존 BP_Player PlaceableMeshes와 같은 메시 지정. BP_Player의 EdgeMeshData와 WBP_TabMenuWidget의 EdgeMeshData 지정. WBP_TabMenuWidget에 ComboBoxString(EdgeMeshComboBox)·Button(BuildWallsButton) 추가, 기존 WBP_BuildingGrid 인스턴스 이름을 BuildingGridWidget으로. 추가 전까지 BindWidget 때문에 WBP 컴파일 오류.
