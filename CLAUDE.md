## 프로젝트: Asteria

- 코옵 어드벤처러 길드 홀 시뮬 (Unreal Engine, C++).
- **핵심 불변조건 — 서버(호스트) 권위 → 클라이언트 동기화.**
- 소스는 **기능별 폴더로 그룹화**. 모듈 루트(`Source/Asteria`) flat 배치 금지.


## 구현 TodoList

* 길드 내 대기 구역 수와, 일부 길드에서 구역 없이 대기할 수 있는 Npc 수를 정한 후, 이를 고려하여 등급별로 길드 내부에 상주할 수 있는 Npc 수를 제약. - Npc의 만료 시간 설정에도 길드가 과포화될 때 구현

* 플레이어의 퀘스트 수락 판단 - Npc가 퀘스트를 정상적으로 수행할 수 있을지를 판단해서 수락 여부를 정해야 함

* 영업 시작, 밤낮 등을 구현해야 함.

* Design 맵 게임 로직 배치
게시판·접근 지점(Counter 칸 2~3개·Lounge 자리·Exit)·NpcSpawner 미배치. GameDefaultMap/EditorStartupMap은 아직 Main (DefaultEngine.ini)

* 코옵 스테이션 분담 — 미착수
2~4인이 홀 안에서 일을 나눠 맡는 구조

* 마법 접목 — 미착수
Docs/Asteria_Magic_System.md 참고

* 효용-AI NPC — 미착수 (후순위)
상황별 점수로 행동을 고르는 NPC. 기억·친밀도·소문 포함

* 길드 자금을 이용해 대기 구역을 늘린다던가, 건물을 확장한다던가 하는 사용처가 필요 - 격자 건물(아래) 완성 후 `SetEdge`/`SetFloor`로 구현

* 격자 건물 — 미착수 (계획은 아래 "격자 건물 구현 계획")


## 격자 건물 구현 계획

건물 벽은 **PCG를 쓰지 않고** 액터 + 인스턴스 메시(ISM)로 직접 배치한다.
- 이유: 배치 규칙이 결정적이라 PCG가 할 일이 없음 / 실행 중 변 단위 편집(길드 확장)이 핵심인데 PCG는 배열 파라미터 전달·전체 재생성·캐시 문제가 있음 / 액터가 변 목록을 직접 복제하면 서버 권위가 단순해짐.

### 데이터 모델
- 정수 좌표의 무한 격자, 한 칸 300. 액터 트랜스폼이 원점(위치·회전을 따름).
- **변 키** = 꼭짓점 `FIntPoint` + `Axis`(0=+X, 1=+Y). 키가 유일해 한 변에 모듈 하나.
- **모듈**: `UBuildingEdgeModule : UDataAsset { UStaticMesh* Mesh; }` — 300×300, 피벗 아래·왼쪽, +X로 뻗고 두께 -Y. 벽·창문·문 모두 모듈 에셋(`DA_Edge_Wall`/`Window`/`Door`). 새 종류는 에셋만 추가. 벽 길이에 따른 창·문 자동 배치 없음.
- `FBuildingEdge { FIntPoint Vertex; uint8 Axis; bool bFlip; UBuildingEdgeModule* Module; }` — `bFlip`은 실내 면(판자·벽기둥 쪽)을 반대로.
- 바닥: `TArray<FIntPoint> FloorCells`(칸 단위).
- 메시는 C++ 문자열이 아니라 에셋·액터 속성으로 참조(이동·이름 변경·쿠킹 누락 문제 해소).

### 배치 규칙 (PCG와 무관한 순수 계산 `BuildingGrid::Build(FloorCells, Edges) → Transforms + Meshes`)
- **바닥 칸**: 판자 바닥, 무늬 방향이 돌지 않게 월드 축 기준.
- **변**: 모듈 메시 + 위에 판자 줄(Plank, Z 300)을 **항상 한 세트**. `bFlip`이면 반대쪽 끝에서 시작해 yaw 180.
- **꼭짓점 – 벽기둥**: 닿는 변마다 실내 면에. 반대편에 실내 방향이 같은 변이 곧게 이어지면 꼭짓점 가운데 1개(칸 경계), 아니면 꼭짓점에서 30 안쪽(벽 끝). L자 모서리는 두 끝 벽기둥이 맞붙음.
- **꼭짓점 – 모서리 기둥**: 같은 방향 직선 연속을 뺀 모든 경우(끝, L, T, 십자, 실내 방향 반전)에 기둥 + 판자 높이(160)로 줄인 기둥. 위치는 축마다 그 축에 수직인 변의 바깥쪽으로 20.
- 부품 치수는 이전 구현 기준: 벽기둥 반폭 30·반두께 6, 모서리 기둥 40×40×300, 벽 높이 300, 판자 160.

### 액터 `ABuildingGrid` (`Source/Asteria/Building/`)
- `bReplicates`, 레벨에 배치. `FloorCells`·`Edges`는 `ReplicatedUsing=OnRep_Layout`.
- 세트 부품 속성: `FloorMesh`, `PlankMesh`, `PilasterMesh`, `ColumnMesh`.
- `SetEdge(Vertex, Axis, Module /*null=제거*/, bFlip)`, `SetFloor(Cell, bOn)` — 서버만. 플레이어 요청(RPC)이 생기면 서버에서 좌표 범위·모듈 유효성 검증.
- `Rebuild()`: 메시별 ISM을 비우고 `Build()` 결과로 다시 채움. `OnConstruction`·`OnRep_Layout`·`Set*`에서 호출. 전체 재생성으로 시작, 편집이 잦아지면 바뀐 변만 갱신.
- 편집: `CallInEditor AddRectRoom()`(`RoomMin`/`RoomMax`로 외곽 벽 + 바닥 채움) 후 `Edges` 배열에서 변 모듈을 창·문으로 교체. `OnConstruction`으로 에디터 미리보기.

### 구현 단계
1. `UBuildingEdgeModule`, `FBuildingEdge`, `BuildingGrid::Build()` + 자동화 테스트 1개(3×2 방: 모서리 기둥 4개, 벽기둥 개수).
2. `ABuildingGrid` 액터(ISM, 복제, `SetEdge`/`SetFloor`, `AddRectRoom`).
3. 모듈 에셋 3종(`SM_Wall_Tavern_D`, `_Window`, `_Entrance_300`) 생성, Design 맵에 배치.

### 주의
- 내비메시가 Static(설정 없음) → 실행 중 벽 변경이 NPC 길찾기에 반영 안 됨. 실행 중 확장 구현 때 Runtime Generation을 `Dynamic`으로.
- T자·십자에서 벽기둥이 가로지르는 벽과 겹칠 수 있음 → 구현 후 화면으로 확인·조정.
- 카운터 PCG도 삭제됨 → 카운터 길이 조절 배치 수단 없음. 격자 모듈로 넣을지 별도 액터로 할지 미정.
