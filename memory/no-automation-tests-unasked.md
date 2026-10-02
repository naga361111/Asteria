---
name: no-automation-tests-unasked
description: 설계·스펙에 자동화 테스트를 사용자가 요청하지 않으면 넣지 않는다
metadata:
  node_type: memory
  type: feedback
  originSessionId: 7e4f38d4-ddba-41e6-abc7-70958c7568fa
  modified: 2026-10-01T14:46:37.222Z
---

설계안이나 구현 단계에 Unreal 자동화 테스트(테스트 케이스·검증 숫자 등)를 임의로 넣지 않는다. 2026-10-01 건물 편집기 설계에서 `BuildingLayout` 자동화 테스트를 넣었다가 "이런거 하라고 안했어"로 전부 빼라는 지적을 받음.

**Why:** 사용자가 요청한 범위만 원함. 테스트도 요청하지 않은 추가 작업으로 본다.

**How to apply:** 스펙·설계 단계 목록에 자동화 테스트 항목을 넣지 않는다. 검증은 빌드·실행 확인으로 하고, 테스트가 필요해 보이면 제안만 한 줄로.
