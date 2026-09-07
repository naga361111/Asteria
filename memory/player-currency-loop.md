---
name: player-currency-loop
description: 플레이어(길드) 재화 earn→spend 루프 완성에 남은 작업 + 미확정 설계 결정
metadata: 
  node_type: memory
  type: project
  originSessionId: 93ac0cfd-f028-44f3-889d-147cf714b646
  modified: 2026-09-07T03:14:21.500Z
---

프로토타입 목표 = **재화 벌이→소모 루프가 도는 것을 증명.** 컨베이어(게시→수주→추상해결→클리어→NPC성장)는 동작하나, **끝단 산출물(재화)이 통째로 비어 있음.** (2026-09-07 분석)

**Why:** 소모처 없는 재화는 루프가 아니라 카운터. earn·spend 양쪽과 그 사이 지급/차감 계약이 다 필요.

**남은 작업 (카테고리):**
- **EARN**: (1) 재화 타입/지갑 자체가 코드에 없음. (2) `FQuest`에 보상 필드 없음. (3) 지급 훅 없음 — `ClearQuest`가 `bIsCleared`만 뒤집고 끝.
- **SPEND**: (4) 소모처(sink) 0개. (5) 구매 트랜잭션+검증 경로 없음(클라 요청→서버 잔액검증→차감→효과).
- **UI**: (6) 잔액 표시 위젯, 소모 UI(고용/상점 버튼).

**미확정 설계 결정 (How to apply — 순서: 소유→방향→검증):**
- **지갑 소유**: 코옵이라 플레이어별 아님 → **길드 공유**, 서버 소유·전원 복제. 위치 = 전용 경제 컴포넌트/서브시스템으로 분리 vs GameState에 얹기(QuestPulls 옆이라 편하나 비대화).
- **보상 = 파생**: `FQuest`에 저장 말고 `(랭크)→보상액` 순수 함수(커브/DataAsset, 서버 소유). 랭크가 보상 스케일 파생([[quest-rank-system]])이라 일관됨. `RequiredRankUpData`와 같은 패턴 → FQuest 필드 안 늘어남.
- **소모처 1개**: 프로토타입은 sink 하나면 루프 닫힘. 추천 = 게시판 리필(돈→새 퀘스트, earn에 되먹임) 또는 모험가 고용(처리량↑). 기존 QuestPulls/NPC 스폰 재활용.
- **지급 시점 = 게이트② 확정 RPC**: 플레이어 완료 확정 핸들러가 상태 전이 + 보상 지급을 한 트랜잭션으로. 별도 훅 불필요. [[npc-quest-approval-flow]] 참고.
- **아이템 체계 선행 여부 (미합의)**: 최소 루프엔 순수 재화면 충분 → 아이템은 "보상 다양화" 단계로 미룰 것 제안함. 사용자 확답 대기.

관련: [[todoList]], [[quest-rank-system]], [[npc-quest-approval-flow]], [[project-direction-guild-sim]]
