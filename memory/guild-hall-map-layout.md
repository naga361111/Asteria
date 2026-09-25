---
name: guild-hall-map-layout
description: "길드 홀 맵(/Game/Map/NewScene) 구조 — 접수처 중심, U자형 복층(바닥 z=430)은 추가 콘텐츠용으로 비워 둠, 천장 z=1100, 좌표·태그 규칙"
metadata:
  node_type: memory
  type: project
  originSessionId: e15c9c14-50e6-409a-85ce-3ac1b9b65d9c
  modified: 2026-09-25T11:23:57.991Z
---

길드 메인 홀 맵은 `/Game/Map/NewScene`이다(2026-09-25 기준, 이름은 아직 정하지 않음). Hearthvale 에셋으로 만들었고, 오브젝트는 전부 Outliner의 `GuildHall/…` 폴더에 들어 있다.

- 홀 안쪽은 x, y 모두 -1200~1200(24m)이다. 입구는 -Y쪽 벽 가운데, 접수처(메인)는 +Y쪽 가운데(x -585~440, y 385~1200)에 있다.
- 높이(2026-09-25에 사용자 요청으로 올림): 1층 층고 400(벽 1단을 400으로 늘림), 복층 바닥 윗면 z=430(아래 들보 370~405), 천장 z=1100. 벽은 0~400 / 400~700 / 700~1000에 나무 띠벽 1000~1100.
- 복층은 U자형이다: 좌(+X) x 600~1200, 우(-X) x -1200~-600(둘 다 y -1200~300), 입구 위 y -1200~-600. 가운데(x ±600, y -600~1200)는 천장까지 트여 있다.
- 쌍둥이 계단은 세 단(한 단 높이 143)이고 x ±(330~592), y 19~-600이다. 입구 위 발코니로 올라간다.
- 복층 바닥은 사용자가 추가 콘텐츠를 넣으려고 일부러 비워 둔 공간이다. 복층 바닥 판에는 태그 `Mezzanine`, `MezzanineZone_E/W/S`가 붙어 있다.
- 자리 표시 태그: `Reception`, `QuestBoard`, `GuildEntrance`, `GuildVault`. 게임 로직 액터(C++ 게시판, NPC 접근 지점·생성기)는 아직 없다.
- PlayerStart는 카운터 안쪽 (-90, 1080, 155), 입구 방향(yaw -90)이다.
- NavMesh 범위는 z -50~850이다. 계단 위 길 찾기는 아직 검증하지 않았다.

**Why:** 사용자가 설계 방향을 정했다. 카운터가 메인이고, 카운터에서 복층이 한눈에 보여야 하며, 복층은 추가 콘텐츠용이다. 이후 층고를 높여 광활한 느낌을 원했다.
**How to apply:** 복층에 콘텐츠를 넣거나 맵을 고칠 때 이 좌표와 태그를 기준으로 하고, 접수처에서 복층이 보이는 시야를 가리지 않는다. 관련: [[project-direction-guild-sim]], [[hearthvale-occlusion-mask]], [[unreal-mcp-set-transform-resets]]
