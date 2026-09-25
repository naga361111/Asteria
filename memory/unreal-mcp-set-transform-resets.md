---
name: unreal-mcp-set-transform-resets
description: unreal-mcp의 ActorTools.set_actor_transform은 빠진 항목(회전·크기·위치)을 기본값으로 초기화함 — 항상 세 항목을 모두 넘길 것
metadata:
  node_type: memory
  type: feedback
  originSessionId: e15c9c14-50e6-409a-85ce-3ac1b9b65d9c
  modified: 2026-09-25T11:01:20.047Z
---

`editor_toolset.toolsets.actor.ActorTools.set_actor_transform`에 location만 넘기면 회전은 0, 크기는 1로 초기화된다. scale만 넘기면 위치가 원점으로 간다. 도구 설명에는 "빠진 항목은 바꾸지 않음"이라고 되어 있지만 실제 동작은 다르다(2026-09-25 확인).

**Why:** 길드 홀 복층 작업 중 이것 때문에 천장 판·들보·기둥·벽난로 구역 97개가 틀어졌다. 원본 레벨에서 변환값을 다시 읽어 와 복구해야 했다.

**How to apply:** 액터를 옮기거나 크기를 바꿀 때는 먼저 get_actor_transform으로 현재 값을 읽고, location·rotation·scale 세 항목을 모두 채워서 넘긴다. 여러 개를 한꺼번에 옮기기 전에는 레벨을 저장해 복구 지점을 만든다. 관련: [[guild-hall-map-layout]]
