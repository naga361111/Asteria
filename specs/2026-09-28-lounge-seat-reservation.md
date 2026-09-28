요청: 설계 (Npc가 대기를 위해 Lounge로 이동할 때 모두 가장 가까운 같은 지점으로 가는 문제 수정 — 해당 자리에 다른 Npc가 있으면 다음으로 가까운 빈 Lounge로 이동. 고르는 시점에 자리를 예약하고 태스크 종료 시 해제)
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/Actors/ApproachPointActor.h / .cpp (수정)
Occupant: 이 지점을 예약한 폰의 약참조 — 서버 전용이라 복제·UPROPERTY 없음, 폰이 사라지면 자동으로 빈 자리 (신규)
FindNearestFree(): 종류가 일치하고 예약되지 않은 지점 중 Origin에서 2D 거리로 가장 가까운 것 반환, 없으면 null — static, BTTask_WaitForConfirmQuest·BTTask_WaitForSettleConfirm의 ExecuteTask()가 호출 (신규)


[2단계]
Source/Asteria/BTNode/BTTask_WaitForConfirmQuest.cpp (수정)
FindNearestWaitPoint(): 익명 네임스페이스의 복제 탐색 함수 삭제 — AApproachPointActor::FindNearestFree()로 대체 (수정)
ExecuteTask(): 대기 지점 탐색을 FindNearestFree()로 바꾸고, 찾으면 즉시 그 지점의 Occupant를 이 Npc로 설정. 빈 지점이 없으면 기존 "지점 없음 → 제자리 대기" 경로 유지 - AApproachPointActor::FindNearestFree() 호출 (수정)
OnTaskFinished(): TargetPoint 초기화 전에 TargetPoint의 Occupant 해제 — 성공·실패·중단 모든 경로 공통 (수정)

Source/Asteria/BTNode/BTTask_WaitForSettleConfirm.cpp (수정)
FindNearestSettleWaitPoint(): 익명 네임스페이스의 복제 탐색 함수 삭제 — AApproachPointActor::FindNearestFree()로 대체 (수정)
ExecuteTask(): 대기 지점 탐색을 FindNearestFree()로 바꾸고, 찾으면 즉시 그 지점의 Occupant를 이 Npc로 설정. 빈 지점이 없으면 기존 "지점 없음 → 제자리 대기" 경로 유지 - AApproachPointActor::FindNearestFree() 호출 (수정)
OnTaskFinished(): TargetPoint 초기화 전에 TargetPoint의 Occupant 해제 — 성공·실패·중단 모든 경로 공통 (수정)
