---
name: quest-npc-architecture
description: "퀘스트/NPC 재설계 아키텍처 — 퀘스트 상태 기계가 단일 지휘자, NPC는 아무것도 안 드는 연주자"
metadata: 
  node_type: memory
  type: project
  originSessionId: edc01ebd-f6ed-4930-b86d-569e068f2b5a
  modified: 2026-09-09T08:04:46.144Z
---

퀘스트/NPC 흐름 재설계 확정 (2026-09-09). 기존 얽힘을 클린 슬레이트로 전부 삭제하고 이 구조로 새로 구현한다.

**핵심 논지:** 퀘스트의 상태 기계가 유일한 지휘자(conductor)다. NPC·플레이어·시스템은 전이를 *요청*하고 이벤트에 *반응*할 뿐, 아무도 상태를 직접 고쳐 쓰지 않는다.

## 확정 사항

1. **QuestService = 퀘스트 상태의 유일 소유자·유일 전이 게이트.** GameState가 소유(인라인, 별도 서브시스템으로 분리 X — 과설계 회피). 서버 권위. 외부엔 intent 메서드(`RequestSelect`/`Approve`/`Clear`)만 노출하고, 성공 시 도메인 이벤트(`OnQuestAccepted`/`OnQuestCleared` 등)를 방출. 퀘스트 배열을 직접 만지는 코드는 이 서비스 밖에 없다.
2. **퀘스트 상태 = 동기화 primitive.** `EQuestType`: (Generated→)Posted→Selected→AwaitingApproval→Accepted→(InProgress→)Cleared. "플레이어가 승인 가능"·"NPC가 진행 가능"은 별도 플래그(구 `bWaitForQuestAccepted`)가 아니라 **퀘스트 상태 그 자체**로 표현.
3. **관계 방향은 퀘스트 쪽 소유.** 퀘스트가 `AssignedNpc`를 기억. **NPC는 퀘스트에 대해 아무것도 안 든다 — id 핸들조차 X.** "내 퀘스트"는 `AssignedNpc == self` 조회로 파생. 선택 시 NPC가 self를 넘기고 서비스가 도장을 찍을 뿐.
4. **NPC는 자기 것만 소유** (rank/level/money 등 진척). 퀘스트 데이터(랭크/보상/상태)는 절대 복사·보관하지 않고 필요할 때 서비스에 질의.
5. **NPC BT = 순수 결정·이동 레이어.** 각 BT 페이즈 ↔ 퀘스트 상태 한 칸이 1:1. "요청→걷기→상태 변화 대기→다음"의 반복. 대기는 델리게이트 구독이 아니라 퀘스트 상태 조건(이벤트→블랙보드→BT 패턴).
6. **플레이어는 NPC가 아니라 카운터에** 승인 의사만 전달 → `QuestService.Approve(questId)`. 카운터가 `IInteractable` 구현.
7. **보상/랭크업/길드 수수료 = `OnQuestCleared` 이벤트 구독으로 반응** (인라인 연쇄 호출 X). NPC 랭크가 오르면 다음 `RequestSelect(myRank)`가 더 높은 일감까지 집어 루프가 닫힌다.

**전제(load-bearing):** NPC당 활성 퀘스트 1개. 이게 무너지면 "내 퀘스트"가 애매해져 NPC가 핸들을 다시 들어야 하므로 3·4번이 붕괴. (※ 강하게 권고했고 방향은 합의됐으나 유저의 명시적 "1개 확정" 발화는 미확인 — 구현 착수 전 재확인 여지.)

**Why:** 기존 얽힘의 근본 원인은 `EQuestType` 전이 소유권이 BTTask/Player/BTTask로 흩어지고, 이를 NPC의 깃발+델리게이트로 억지 동기화한 것. 전이를 한 곳에 모으고 나머지는 상태를 *읽기만* 하면 굵은 흐름이 단순·명확해진다. 모든 결정 로직은 서버 한 곳(AI도 서버 실행 → BT가 권위 상태 직접 읽고 요청), 클라는 순수 뷰(복제 상태로 UI만). 상태 중복 금지 → 어긋날 여지 제거.

**How to apply:** GameState에 QuestService(QuestPulls 배열 + 전이 메서드 + 이벤트) 세움. Quest 데이터 모델에 `AssignedNpc` 추가, 상태 enum 확장. 카운터 액터가 `IInteractable` 구현해 Approve 위임. 랭크는 기존 `ERank` 재사용([[quest-rank-system]]). 소스는 기능별 폴더 그룹화([[script-folder-organization]]). 진행 현황은 [[todoList]]. 방향성 [[project-direction-guild-sim]].
