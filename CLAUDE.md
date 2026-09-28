## 프로젝트: Asteria

- 코옵 어드벤처러 길드 홀 시뮬 (Unreal Engine, C++).
- **핵심 불변조건 — 서버(호스트) 권위 → 클라이언트 동기화.**
- 소스는 **기능별 폴더로 그룹화**. 모듈 루트(`Source/Asteria`) flat 배치 금지.


## 구현 TodoList

* 접근 지점 자리 점유 — 미착수
Lounge(대기 자리)·Counter(창구 칸)는 한 지점에 NPC 하나. 지점이 점유 NPC를 약한 참조로 기억, 빈 지점만 탐색.
NPC는 한 번에 한 자리만 가짐(새 지점 출발 시 이전 자리 해제, 소멸 시 자동 해제). 빈 칸 없으면 실패하지 말고 빈 칸 날 때까지 대기(실패 시 BT가 게시판부터 재시작). Dungeon·Exit은 공유

* 번호표식 대기 연출 (BT_Npc 재구성) — 미착수
창구 제출 → 빈 Lounge 자리에서 대기 → 플레이어 컨펌 시 창구 칸으로 와서 확인(엔진 Wait 1~2초) → 던전/출구.
대기는 Simple Parallel(주: WaitForConfirmQuest/WaitForSettleConfirm, 배경: MoveToApproachPoint(Lounge)). 실제 처리(수주 확정·수수료 입금)는 지금처럼 컨펌 순간 서버에서, 창구로 오는 건 연출만

* NpcSpawner 주기 생성 — 미착수
현재 BeginPlay에서 1회만 생성. 일정 시간마다 생성하되 빈 Lounge 자리가 있고 집을 퀘스트가 있을 때만(자리 수 = 인원 상한).
NpcId가 에디터 기본값 고정(0) → 스폰 시 고유 번호 부여. 퀘스트를 못 집은 NPC는 Despawn으로 퇴장

* NPC 등급 — 스폰 시 부여
NpcRnk(AsteriaNpc.h)가 에디터 설정값 고정. NPC가 일회성이라 성장 대신 스폰 시 등급 부여(퀘스트 등급처럼 길드 등급 가중 등)

* Design 맵 게임 로직 배치
게시판·접근 지점(Counter 칸 2~3개·Lounge 자리·Exit)·NpcSpawner 미배치. GameDefaultMap/EditorStartupMap은 아직 Main (DefaultEngine.ini)

* 코옵 스테이션 분담 — 미착수
2~4인이 홀 안에서 일을 나눠 맡는 구조

* 마법 접목 — 미착수
Docs/Asteria_Magic_System.md 참고

* 효용-AI NPC — 미착수 (후순위)
상황별 점수로 행동을 고르는 NPC. 기억·친밀도·소문 포함