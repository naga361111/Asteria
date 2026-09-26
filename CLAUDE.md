## 프로젝트: Asteria

- 코옵 어드벤처러 길드 홀 시뮬 (Unreal Engine, C++).
- **핵심 불변조건 — 서버(호스트) 권위 → 클라이언트 동기화.**
- 소스는 **기능별 폴더로 그룹화**. 모듈 루트(`Source/Asteria`) flat 배치 금지.


## 구현 TodoList

* 길드 금고 UI — 신규
GuildFunds(AsteriaGameState)는 복제되지만 화면 표시가 없음. 09-09 재작성 때 기존 HUD 삭제됨

* NPC 등급 성장 — 재구현
NpcRnk(AsteriaNpc.h)가 에디터 설정값 고정. 09-05 구현분이 09-09 재작성 이후 미복구

* Design 맵 게임 로직 배치
게시판·접근 지점·NpcSpawner 미배치. GameDefaultMap/EditorStartupMap은 아직 Main (DefaultEngine.ini)

* AsteriaPlayer.cpp — 창구 위치 서버 검증
플레이어가 실제로 창구 앞에 있는지 서버가 확인하지 않음 (Server_AcceptQuestAssignment TODO)

* BTTask_WaitForConfirmQuest.cpp — AssignmentId 전달
SelectQuest가 AssignmentId를 넘겨주면 Assignment 전체 순회 제거

* 코옵 스테이션 분담 — 미착수
2~4인이 홀 안에서 일을 나눠 맡는 구조

* 마법 접목 — 미착수
Docs/Asteria_Magic_System.md 참고

* 효용-AI NPC — 미착수 (후순위)
상황별 점수로 행동을 고르는 NPC. 기억·친밀도·소문 포함