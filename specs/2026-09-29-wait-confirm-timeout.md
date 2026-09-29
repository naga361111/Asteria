요청: WaitForConfirmQuest에 제출 후 대기가 시작되도록 해줘. 게임 시간으로 30분이 지나면 해당 노드는 실패해. 대기 시간은 BT 노드의 디테일 패널에서 수정할 수 있어야 해.
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/BTNode/BTTask_WaitForConfirmQuest.h / .cpp (수정)
WaitGameMinutes: 컨펌 대기 제한 시간(게임 분), 기본값 30 — BT 노드 디테일 패널에서 수정, Category "Wait", 최소 1 (신규)
GameClock: 마감 확인용 GameClockService 약한 참조 — ExecuteTask()가 설정, TickTask()가 읽음, OnTaskFinished()가 초기화 (신규, 추가)
ExpireGameMinute: 이번 실행의 대기 마감 게임 분 — ExecuteTask()가 설정, TickTask()가 비교, OnTaskFinished()가 초기화 (신규)
ExecuteTask(): 내 Submitted Assignment를 찾은 직후, 대기 지점 탐색·이동 요청보다 먼저 "지금 게임 분 + WaitGameMinutes"로 마감 시각을 잡음 — 제출 직후 실행되므로 대기는 제출 시점부터 시작, 지점 없음·이동 실패로 제자리 대기하는 경로에도 마감이 적용됨 - UGameClockService::GetGameMinutes() 호출 (수정)
GameClockService 없음: GameState에 GameClockService가 없으면 마감을 잡을 수 없으므로 Failed (신규, 추가)
이미 컨펌됨·대기 대상 없음 경로: 대기하지 않고 즉시 끝나므로 마감을 잡지 않음 — 기존 동작 유지 (재사용: Source/Asteria/BTNode/BTTask_WaitForConfirmQuest.cpp)
TickTask(): 회전 여부와 관계없이 매 틱 먼저 마감을 확인해 지금 게임 분 ≥ ExpireGameMinute면 경고 로그 후 Failed로 종료, 아니면 기존 회전 페이즈 처리 — 이동 중이든 도착 후든 적용 - UGameClockService::GetGameMinutes(), FinishLatentTask() 호출 (수정)
OnTaskFinished(): 기존 정리에 더해 GameClock·ExpireGameMinute 초기화 — 시간 초과 Failed도 이 경로로 와서 구독 해제·이동 정지·대기 지점 예약 해제가 그대로 일어남 (수정)
