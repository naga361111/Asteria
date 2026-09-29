## 프로젝트: Asteria

- 코옵 어드벤처러 길드 홀 시뮬 (Unreal Engine, C++).
- **핵심 불변조건 — 서버(호스트) 권위 → 클라이언트 동기화.**
- 소스는 **기능별 폴더로 그룹화**. 모듈 루트(`Source/Asteria`) flat 배치 금지.


## 구현 TodoList

* 길드 내 대기 구역 수와, 일부 길드에서 구역 없이 대기할 수 있는 Npc 수를 정한 후, 이를 고려하여 등급별로 길드 내부에 상주할 수 있는 Npc 수를 제약. - Npc의 만료 시간 설정에도 길드가 과포화될 때 구현

* 플레이어의 퀘스트 수락 판단 - Npc가 퀘스트를 정상적으로 수행할 수 있을지를 판단해서 수락 여부를 정해야 함

* 영업 시작, 밤낮 등을 구현해야 함.

* Design 맵 게임 로직 배치
게시판·접근 지점(Counter 칸 2~3개·Lounge 자리·Exit)·NpcSpawner 미배치. GameDefaultMap/EditorStartupMap은 아직 Main (DefaultEngine.ini)

* 코옵 스테이션 분담 — 미착수
2~4인이 홀 안에서 일을 나눠 맡는 구조

* 마법 접목 — 미착수
Docs/Asteria_Magic_System.md 참고

* 효용-AI NPC — 미착수 (후순위)
상황별 점수로 행동을 고르는 NPC. 기억·친밀도·소문 포함

* 길드 자금을 이용해 대기 구역을 늘린다던가, 건물을 확장한다던가 하는 사용처가 필요 - 추후 PCG로 건물을 짓게 되면 구현