---
name: spec-show-before-askuserquestion
description: /spec 승인 단계에서 AskUserQuestion 직전 텍스트가 사용자에게 안 보임 — 스펙을 출력하고 턴을 끝내 텍스트로 승인을 물을 것
metadata:
  node_type: memory
  type: feedback
  originSessionId: 2be34011-343a-4a2b-b3a0-af3afc8a4522
  modified: 2026-10-01T12:31:56.241Z
---

/spec 6단계에서 스펙 yaml을 출력한 뒤 같은 응답에서 AskUserQuestion을 부르면 사용자는 스펙을 보지 못하고 "스펙을 보여줘야지"라고 답한다(2026-10-01 두 번 발생).

**Why:** 이 앱(Code 탭)에서는 AskUserQuestion 직전의 텍스트가 질문 대화창에 가려져 안 보이는 것으로 보인다.

**How to apply:** 스펙 경로와 yaml 블록을 출력하고 "구현 진행 / 중단"을 텍스트로 물은 채 턴을 끝낸다. AskUserQuestion은 쓰지 않는다.
