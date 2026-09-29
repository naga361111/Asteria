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

## 개선점 (2026-09-29 길드 건물 PCG 리팩터링 검증)

3. [확장성] 건물이 월드 원점 고정 — `PCGGuildShellSettings.cpp` 노드가 볼륨 위치·회전을 무시. 해결: PCG 실행 정보(ExecutionSource)의 볼륨 트랜스폼/경계 최소 모서리를 모든 점에 적용.
4. [개발 편의] C++ 상수 변경 후 Live Coding 해도 PCG 캐시로 옛 결과 — 해결: `FPCGGuildShellElement`에 `IsCacheable` → false.
5. [검증] 리팩터링 스냅샷이 `Saved/`(git 제외)에만 있고 스크립트 없음 — 해결: `FGuildShellBuilder` 크기·문 조합별 메시 개수 자동 테스트(CQTest).
6. [중복] `GuildShellInterior.cpp` BuildDecor의 `Side == 0 ? … : …` 분기 11개 → `AddOnWall`로 통일. 문·창 구간(Openings)을 생성자에서 계산해 BuildWalls 순서 의존 제거. 주의: 벽 깃발은 양쪽 yaw 0이라 바꾸면 B면이 180° 돎 — 의도 확인 먼저.
7. [의존성] `MaxCells`가 PCG 노드 클래스에 있어 서비스가 PCG 헤더를 끌어옴 → `GuildShell::MaxCells`(GuildShellBuilder.h)로 이동.
8. [사소] 노드의 문 속성 8개+MakeDoor → `FGuildShellDoor` 4개(UE 5.8 구조체 필드 오버라이드, 그래프 핀 재연결 필요, 선택). `GuildShellDoor.h` "그래프가 멈춘다" 주석 → 노드가 멈춤.

참고(결함 아님): 문 설정은 복제 안 됨(실행 중 건물 확장 시 서비스가 함께 복제해야 함). 메시 경로가 C++ 문자열이라 에셋 이동·이름 변경 자동 반영 안 되고, 패키징 때 저장 레벨에 안 쓰인 메시가 빠질 수 있음 → 건물 메시 폴더를 쿠킹 목록에 추가.
