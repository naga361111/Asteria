---
name: quest-rank-system
description: 퀘스트 랭크 등급 체계 확정 — F~S 7단계 알파벳
metadata: 
  node_type: memory
  type: project
  originSessionId: 00973789-d68f-4c7d-a08d-73ea36c356c1
  modified: 2026-09-05T06:07:43.376Z
---

퀘스트 랭크 등급은 **F~S 7단계 알파벳**으로 확정 (2026-09-05 결정).

`EQuestRank : uint8 { F=0, E=1, D=2, C=3, B=4, A=5, S=6 }` — 낮은 값 = 낮은 랭크.

등급 성격: F 튜토리얼/잡무 → E 초급 → D 초중급(파티 권장) → C 중급(코옵 밸런스 기준점, 스케일 1.0) → B 상급 → A 최상급(엔드 초입) → S 특수/보스(단발성).

**Why:** 모험가 길드 장르 관습이라 학습 부담이 없고, 인덱스 기반으로 난이도/보상 커브 계산이 쉬움. 초반 스코프에 7단계가 적정(5는 성장감 부족, 9는 과함).

**How to apply:** `FQuest`에 `EQuestRank Rank` 필드 추가. 랭크가 (1) NPC 수령 조건(NPC 레벨 ≥ 요구 레벨), (2) 보상 스케일, (3) 게시판 등장 빈도를 파생. 확장 시 SS/EX를 enum 뒤에 append-only로 붙여 직렬화/네트워크 값 보존. 모든 데이터는 서버(호스트) 소유 후 클라 동기화. 관련: [[todoList]], [[project-direction-guild-sim]]
