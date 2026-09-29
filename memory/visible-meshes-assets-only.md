---
name: visible-meshes-assets-only
description: 눈에 보이는 곳엔 엔진 기본 도형(Cube 등) 대신 에셋(Hearthvale 등)만 쓰기 — 외관도 항상 신경써야 함
metadata:
  node_type: memory
  type: feedback
  originSessionId: 7922e10b-a6e2-4d6d-9487-1aef4f271de6
  modified: 2026-09-29T07:34:34.841Z
---

2026-09-29 PCG 천장이 빛을 막도록 엔진 기본 큐브를 막음판으로 얹었더니, 사용자가 이렇게 지시했다. "외견도 신경써야 해. 눈에 보이는 곳에 저렇게 쓰지 마. 눈에 보이는 곳에는 에셋만 활용해"

**Why:** 기능(빛 차단·충돌)을 맞춰도 외관을 해치면 안 된다. 밖이나 위에서 보이는 면도 외관에 포함된다.
**How to apply:** 보이는 표면은 에셋 메시로만 만든다. 기능용 부품이 필요하면 그 기능도 에셋으로 해결한다. 예: 한쪽 면 판재는 양면 그림자로 설정하고 충돌을 켜고, 위를 보는 판재를 한 겹 더 깐다. 캡처는 안과 밖 모두 확인한다. 관련: [[hearthvale-module-sizes]]
