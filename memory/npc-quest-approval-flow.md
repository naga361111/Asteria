---
name: npc-quest-approval-flow
description: "NPC 퀘스트 플로우를 \"NPC 제안 → 플레이어 승인/확정\" 게이트 2개로 재설계 (미착수, 설계 결정 미확정)"
metadata: 
  node_type: memory
  type: project
  originSessionId: 93ac0cfd-f028-44f3-889d-147cf714b646
  modified: 2026-09-07T03:14:00.732Z
---

플레이어 경험을 위해 NPC 행동 루틴을 바꾼다. **핵심: NPC가 제안(propose)하고 플레이어가 결정(dispose).** (2026-09-07 방향 확정, 구현 미착수)

**Why:** 직업 시뮬이라 NPC의 자율 퀘스트 선택은 맞지만, 최종 수주·클리어 확정 권한은 플레이어에게 줘야 관리자(A형) 경험이 성립. 서버 권위 불변조건과도 정렬.

**새 라이프사이클 (플레이어 게이트 2개 삽입):**
`Posted → [NPC선택]Proposed(=예약) → [플레이어 승인]게이트① InProgress → [타이머]AwaitingConfirm → [플레이어 확정]게이트② Cleared`
게이트①②는 구조 동일: 카운터로 이동 → 대기열 등록 → 플레이어 결정 대기 → resume.

**현재 코드와의 충돌 (반드시 분리해야 함):**
- `AsteriaGameState::GetQuest`가 NPC 선택 순간 `bIsAccepted=true`를 박음 → 선택(select)과 수주(accept) 합쳐져 있음. 분리 필요.
- `BTTask_DoQuest`가 타이머 종료 시 바로 `ClearQuest` 호출 → 완료와 확정 사이에 플레이어 게이트 자리 없음.
- BT 대기 수단이 타이머뿐. 새 게이트는 타이머가 아니라 플레이어 입력(외부 이벤트)을 기다려야 함.

**미확정 설계 결정 (How to apply — 순서: 소유→방향→검증):**
- **A. 상태 모델**: bool 3개(bIsPosted/bIsAccepted/bIsCleared) → `EQuestState` enum 상태머신으로 승격 여부. 상태 5개·선형 전이라 bool은 불법 조합 폭증. enum 권장. **골격이므로 먼저 확정.**
- **B. 카운터 대기열 소유**: `{NPC, QuestId, 게이트종류}` 엔트리의 서버 소유·클라 열람 대기열을 어디 둘지 — 전용 Counter 액터/컴포넌트(추천) vs GameState. UI 바인딩 대상.
- **C. resume 계약**: BT 게이트 태스크가 플레이어 RPC를 어떻게 기다려 깨어나나 — NPC/Controller 델리게이트 bind→FinishLatentTask(추천) vs Blackboard observer abort.
- **D. 게이트 통합**: `BTTask_AwaitPlayerDecision(EGateType)` 하나로 통합 vs 태스크 2개.
- **E. 거절 경로**: 승인 게이트는 승인/거절 둘 다(거절 시 Posted 복귀?), 확정 게이트는 확정만.
- **예약(reservation)**: Proposed 상태가 소프트 락 겸함 → NPC 간 중복 제안 방지(A와 묶임).

**재화 루프와의 수렴점:** 게이트②(완료 확정 RPC)가 곧 길드 보상 지급의 권위적 순간. 별도 지급 훅 불필요 — 확정 핸들러 = 상태 전이 + 보상 지급 한 트랜잭션. [[player-currency-loop]] 참고.

관련: [[quest-rank-system]], [[todoList]], [[project-direction-guild-sim]]
