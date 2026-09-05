---
name: script-folder-organization
description: "Source scripts must be grouped into folders, never dumped flat in the module root"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 5091fc11-fa3c-46b1-ac14-488fa5ff03e3
  modified: 2026-08-25T05:40:22.449Z
---

새 소스 스크립트는 항상 폴더를 만들어 체계적으로 배치한다. 모듈 루트(`Source/Asteria/`)에 파일을 평면으로 흩뿌리지 않는다.

**Why:** 사용자가 명시적으로 요청한 작업 방식 — 파일이 늘어날 때 구조가 유지되도록.

**How to apply:** 기능/역할 단위로 서브폴더를 만든다. 현재 NPC 구조는 Body/Brain 분리를 반영 — `Source/Asteria/NPC/`(캐릭터), `Source/Asteria/NPC/Body/`(상태 컴포넌트), 향후 `NPC/Brain/`(행동 선택 로직). Asteria 모듈은 Public/Private 분리를 안 쓰므로 서브폴더가 include 경로에 자동 포함되어 bare include가 그대로 동작한다.
