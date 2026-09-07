---
name: todolist
description: 할 일 목록
metadata: 
  node_type: memory
  type: project
  originSessionId: 93ac0cfd-f028-44f3-889d-147cf714b646
  modified: 2026-09-07T03:14:33.097Z
---

**모든 시스템은 서버-호스트 동기화를 기본 전제로 한다.**

## 프로토타입 완성 — 현재 두 작업 스트림 (2026-09-07)

1. **재화 earn→spend 루프 완성** → 상세 [[player-currency-loop]]
   - EARN(재화타입/지갑·보상파생·지급훅), SPEND(소모처·구매검증RPC), UI(잔액·소모).
2. **NPC 플로우: 제안→플레이어 승인/확정 게이트 2개** → 상세 [[npc-quest-approval-flow]]
   - 상태 enum 승격, 카운터 대기열, BT resume 계약, 거절 경로.
   - 게이트②(완료 확정)가 재화 지급점 → 두 스트림 수렴.

## 이후 (프로토타입 이후)
- 퀘스트 종류/보상 다양화 (선행: 아이템 체계 — 최소 루프엔 불필요, 뒤로 미룸)
- 살아있는 효용-AI NPC(IAUS·Engram·친밀도·소문) — [[project-direction-guild-sim]]