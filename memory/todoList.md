---
name: todolist
description: 할 일 목록
metadata: 
  node_type: memory
  type: project
  originSessionId: 93ac0cfd-f028-44f3-889d-147cf714b646
  modified: 2026-09-09T08:05:05.419Z
---

**모든 시스템은 서버-호스트 동기화를 기본 전제로 한다.**

## 현재 상태 — 클린 슬레이트 (2026-09-09)

리팩토링 브랜치에서 기존 퀘스트/NPC/재화 구현을 **전부 삭제**함(거의 완성 상태였던 루프를 아키텍처 정리를 위해 의도적으로 백지화). 남은 골격:
- **플레이어**: 이동/시야(Enhanced Input) + BoxComp 오버랩으로 `IInteractable` 대상 감지 + Interact 키.
- **NpcAIController**: 도착 감속 이동 + `RunBehaviorTree`. (퀘스트 블랙보드 타깃 제거됨)
- **빈 셸**: `AAsteriaNpc`(순수 ACharacter), `AAsteriaGameState`, `AAsteriaGameModeBase`.
- **보존된 인프라**: `IInteractable`(구현체 0), `ERank`(F~S).
- 콘텐츠 애셋(.uasset)은 미정리 — 삭제된 클래스 참조로 에디터 경고 예상.

## 다음 — [[quest-npc-architecture]] 대로 재구현

1. GameState에 **QuestService** — QuestPulls 배열 + 단일 전이 게이트(`RequestSelect`/`Approve`/`Clear`) + 도메인 이벤트.
2. **Quest 데이터 모델** 재정의 — `AssignedNpc` 포함, 상태 enum 확장(…AwaitingApproval→Accepted…).
3. **NPC BT** — 페이즈 ↔ 퀘스트 상태 1:1, NPC는 퀘스트 무소유(파생 조회). 대기는 이벤트→블랙보드 조건.
4. **카운터 액터**(`IInteractable`) — 플레이어 승인 진입점 → `QuestService.Approve`.
5. **재화/랭크업/길드 수수료** — `OnQuestCleared` 구독으로 반응(구 재화 루프도 이 안으로 흡수).
6. **퀘스트 게시/목록 UI** 재구축 — 구 TileView 위젯(SystemQuestTile 등) 삭제됨, 새 QuestService 계약에 맞춰 신설.

## 이후 (프로토타입 이후)
- 퀘스트 종류/보상 다양화 (선행: 아이템 체계 — 최소 루프엔 불필요, 뒤로 미룸)
- 살아있는 효용-AI NPC(IAUS·Engram·친밀도·소문) — [[project-direction-guild-sim]]
