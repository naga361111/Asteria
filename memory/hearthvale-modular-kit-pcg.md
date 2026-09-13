---
name: hearthvale-modular-kit-pcg
description: "Hearthvale 킷 그리드 계약(300/층고 300/Tavern_C) 확정 + PCG_BuildingShell 셸 제너레이터 구조·소유 경계"
metadata: 
  node_type: memory
  type: project
  originSessionId: bd81bc78-5d15-43fa-ac3d-477f4f701136
  modified: 2026-09-10T04:10:00.000Z
---

`Content/Hearthvale/`는 외부 반입 모듈러 환경 애셋 팩("Tavern and Homes"류). 순수 아트 계층, 게임플레이 로직 없음. `Hearthvale/` 한 폴더에 격리 유지(vendored 경계).

**그리드 계약 (2026-09-10 실측으로 확정).** 메시 바운즈를 직접 측정해 결정:
- 모듈 300 / 하프 150
- **층고 = 300**, 벽 세트 = **`SM_Wall_Tavern_D`**. 근거는 개구부와 계단이다:
  - 창·문은 **D 세트에만** 있고 전부 300 높이 (`_Window`, `_Window_Thin`, `_Entrance_300`(300 모듈), `_Entrance`/`_Entrance_Dbl`(450 모듈)).
  - 계단 상승량이 300 (`SM_Stairs_Wooden_Wall_A_400` = 400 run x 300 rise). `SM_Stairs_Wooden_A_100_Wide`(rise 100 / run 133.3 / 폭 300) **3장 = 정확히 한 층**.
  - 코너 포스트는 `SM_Wall_House_A_Column`(25 x 60 x **305**). 45도로 돌려 세우면 맞댄 벽 이음매를 양축으로 가림.
- **함정: 400 높이 C 세트(`SM_Wall_Tavern_C`)는 코너 조각이 완비돼 있지만 창·문이 하나도 없다.** 처음에 "코너 조각 유무"로 400을 골랐다가 뒤집었음 — 코너는 기둥으로 가리면 되니 구속조건이 아니었고, 진짜 구속조건은 개구부와 계단이었다. C 세트는 별도 스타일(석조 담/홀)로만 취급할 것.
- `SM_Wall_House_A`는 폭 320이라 그리드 이탈 — 구조용 금지.
- 피벗 규약: 벽 = 한쪽 끝 X=0, 몸통은 로컬 -Y(약 -40~+5). 바닥 타일 `SM_Floor_Tiles_300` = 코너 피벗 [0,300]x[-300,0]. 빔 `sm_beam_wide_300` = X=0 시작, Y·Z 중앙. 계단 모듈은 로컬 -X/-Y로 자람.

**`/Game/PCG/Building/` — 셸 제너레이터 (창·문·계단까지 동작 검증 완료).**
- `PCG_BuildingShell` 그래프 / `BP_BuildingShell`(닫힌 스플라인 `Footprint` + PCGComponent, 클래스 기본 태그 `BuildingFootprint`) / `BP_StairCore`(2점 스플라인, 클래스 기본 태그 `StairCore`).
- **입력 계약 3가지:**
  1. 건물 = 닫힌 스플라인 1개, 꼭짓점은 300 그리드 스냅. 300 배수가 아닌 엣지는 일부러 틈이 남게 둬서 계약 위반이 보이게 함.
  2. **세그먼트 0(스플라인 첫 점에서 나가는 엣지) = 정면.** 1층 정면에만 문법 `[W][D][W,N]*`, 나머지는 전부 `[W,N]*`. 문 위치는 저작되는 값이지 추론되는 값이 아님.
  3. **계단은 별도 액터.** 점 0 = 계단 발치, 방향 = 올라가는 방향. 슬래브가 계단에 반응해 구멍을 뚫지, 그 반대가 아님.
- 검증 수치(1800x1200, 2층): 벽 20 / 창 19 / 문 1 / 기둥 8 / 빔 40 / 계단 3단 / 바닥 1층 24·2층 22(계단참 2셀 제거).
- 벽 레인은 `Spline to Segment` → `Subdivide Segment`(문법) → `Merge Points` → 심볼별 `Filter Attribute Elements` → 스포너 3개. 층 스택은 `Duplicate Point`(Z+300).

**킷 애셋 함정 (바닥/천장/계단):**
- **바닥 타일은 위에서만 렌더된다.** `SM_Floor_Tiles_300`은 두께 0, `_Extrude`(두께 3.4)조차 아래에서 안 보임 → 아래층에서 보면 천장이 없다. **스케일 Z=-1로 뒤집는 건 해결이 아니다** (와인딩까지 뒤집혀 그대로 컬링됨; 디스크립터 `bReverseCulling`도 ISM에 전달 안 됨). 정답은 **X축 180도 회전**(행렬식 +1이라 컬링 불변) + Y -300 보정으로 같은 셀 위에 윗면을 아래로 깔기. 슬래브 인스턴스가 2배가 되므로 나중엔 전용 천장 메시가 낫다.
- **바닥 타일에는 콜리전이 없다.** 트레이스가 전부 통과함. 2층을 걸어다니려면 메시에 콜리전을 만들거나(벤더링 애셋 수정) ISM을 Complex-as-Simple로 돌려야 함 — **미해결, 결정 필요.**
- **계단 run 400 vs 그리드 300 충돌.** 킷 계단 기울기는 고정 0.75라 한 층(300)을 오르면 run이 항상 400이다. 300 그리드에서는 어떻게 놓아도 셀 경계를 100 넘는다. → **계단실은 2셀(600), 도착부에 200짜리 착지참**이 구조적으로 강제된다. 현재 `SM_Floor_Tiles_300_Extrude`를 X 0.667로 스케일해 착지참으로 쓴다.

**PCG 함정 (전부 직접 부딪힘):**
- **`Get Spline Data`의 componentSelector(ByTag)는 동작하지 않는다.** 한 액터에 스플라인 2개를 넣고 컴포넌트 태그로 가르려다 실패 → 계단을 별도 액터로 분리해서 해결. 결과적으로 경계도 더 좋아짐.
- **`Difference`/`Intersection`은 점 중심이 아니라 바운드로 자르고, 스티프니스 0.5면 유효 반경이 바운드의 정확히 2배.** 그래서 `HoleBox` 바운드를 의도한 크기의 **절반**으로 넣어야 원하는 셀만 빠진다. (`Bounds Modifier`의 `bAffectSteepness`를 켜는 변경 직후 에디터가 무한 루프에 빠졌으므로 건드리지 말 것.)
- `Spline Sampler`를 `Subdivision / subdivisionsPerSegment=0`으로 쓰지 말 것. 대신 `Spline to Segment` 사용.
- 인터리어 샘플링은 셀이 아니라 **격자 노드(n+1)** 를 뱉는다 → `+150,+150` 시프트로 셀 중심을 만들고 `Create Surface From Spline` + `Intersection`으로 바깥 행/열 컷. 컬링은 반드시 타일 중심 기준(피벗 오프셋을 먼저 걸면 반 모듈 어긋남).
- 스포너는 `bSynchronousLoad = true`로. 안 그러면 아직 로드 안 된 메시를 조용히 건너뛴다.

**소유 경계 (유지):**
- PCG 소유: 벽 띠·바닥·기둥·상단 빔 (반복 규칙 명확, 재생성해도 게임플레이 불변)
- 수동/스플라인 태그: 문·창 개구부, 계단 위치 (게임플레이 동선 의미)
- **절대 PCG 밖: QuestBoard 등 상호작용물·네비메시** — 서버 권위 대상. 아트 재생성이 게임플레이 상태를 흔들면 [[project-direction-guild-sim]]의 서버 권위 불변조건과 충돌.
- 콜리전은 셸의 일부라 서버·클라가 반드시 같아야 함 → 런타임 생성 금지, 에디터에서 생성해 레벨에 직렬화(현재 `GenerateOnLoad`).

**에디터 함정:** PCG 에디터 생성은 MCP 프로퍼티 더티만으로는 안 돌고, 디테일 패널의 **Generate 버튼을 실제로 눌러야** 큐가 처리됨. `PCGToolset.ExecuteGraphInstance`는 PCGVolume에만 통함.

**에디터/MCP 운용 (매 세션 반복됨):**
- 새 에디터 세션마다 콘솔에 **`Slate.bAllowThrottling 0`** 를 넣어야 PCG 생성 큐가 돈다. 안 넣으면 Generate를 눌러도 `bDirtyGenerated`만 true인 채 아무 일도 안 일어남.
- MCP로는 PCGVolume의 디테일 패널 **Generate 버튼을 실제 클릭**해야 생성된다. 프로퍼티만 더티시켜서는 안 되고, `ExecuteGraphInstance`는 멈춘다. 버튼 ref는 패널이 갱신될 때마다 바뀌므로 **탐색과 클릭을 한 스크립트 안에서** 할 것(따로 하면 Cleanup을 누르게 됨).
- **레인 하나 추가 → 생성 검증 → 애셋 저장**을 반복할 것. 이걸 어기고 세 레인을 한 번에 붙였다가 에디터가 무한 루프에 빠져 작업을 통째로 날렸다.
