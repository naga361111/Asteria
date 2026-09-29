---
name: hearthvale-module-sizes
description: "Hearthvale 건축 부품 실측(2026-09-29) — 300 격자, 벽 피벗 규칙, 층 높이 400+300, 계단 조합. PCG 건물 생성의 기준값"
metadata:
  node_type: memory
  type: project
  originSessionId: 2bd14ed1-a135-4872-89e0-8e0d50adff5f
  modified: 2026-09-29T03:28:24.574Z
---

2026-09-29 에디터 get_bounds로 잰 값. PCG 건물 생성은 `/Game/Map/PCG` 레벨에서 한다.

- 격자 300. 벽 폭 300 / Half 150 / Extend 450 / Long 600. 바닥 Tiles_300·PlanksFloor_A 300×300(두께 거의 0).
- 벽 피벗: 아래·왼쪽 끝, +X로 뻗음, 두께는 -Y쪽(약 40~50). 바닥 피벗: 모서리, +X·-Y로 뻗음. 기둥은 중심 피벗.
- 높이: Tavern_C(돌, 모서리 Corner_150·Corner_in_150 있음) 400 / Tavern_A·B·D·House_A·StoneBrick 300 / Wooden_Planks 160(칸막이).
- A·B·D에는 모서리 부품이 없어 이음새를 기둥(House_A_Column 25~40)으로 가려야 함.
- 예외: PlanksFloor_A_Half는 300×164(150 아님). Entrance/Dbl은 450, Entrance_300은 300.
- 계단: Wooden_A_150_Wide(수평 211, 폭 306, 높이 157), 100_Wide(133, 306, 100). 층고 400 = 150+150+100.
- 바운드는 장식 돌출 포함(Long이 625 등) — 배치는 공칭 크기로.

**Why:** PCG 격자·벽 방향 규칙을 이 값에 맞춰야 이음새가 맞는다.
**How to apply:** 재측정 대신 이 값을 쓰고, 벽 안/밖 면 방향은 아직 눈으로 확인 안 함. 관련: [[guild-hall-map-layout]], [[hearthvale-occlusion-mask]]
