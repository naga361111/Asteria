---
name: hearthvale-occlusion-mask
description: Hearthvale 재질의 탑다운용 투명화(Add Occlusion MAsk)를 1인칭용으로 꺼 둠 — 에셋 폴더는 git 미추적이라 재설치 시 다시 꺼야 함
metadata:
  node_type: memory
  type: project
  originSessionId: e15c9c14-50e6-409a-85ce-3ac1b9b65d9c
  modified: 2026-09-25T06:17:46.190Z
---

Hearthvale 에셋 팩의 M_Tiles 기반 재질 인스턴스 26개에서 `Add Occlusion MAsk` 스태틱 스위치를 false로 바꿨다 (2026-09-25, 길드 홀 맵 작업 중).

**Why:** 이 기능은 탑다운 게임용으로, 카메라와 MPC_PlayerLOcation의 PlayerLocation(기본값 월드 원점) 사이 반경 150 이내의 벽·바닥을 디더링으로 뚫는다. Asteria는 1인칭이라 화면 가운데(원점 방향)에 구멍이 생겼다.

**How to apply:**
- `Content/Hearthvale/`는 .gitignore 대상이라 이 변경은 git에 없다. 에셋 팩을 다시 받거나 다른 PC에서 열면 같은 스위치를 다시 꺼야 한다.
- 새 Hearthvale 재질 인스턴스를 만들 때도 부모가 M_Tiles면 이 스위치를 확인할 것.
- 길드 홀 맵은 `/Game/Map/NewScene` ([[project-direction-guild-sim]]).
