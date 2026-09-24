요청: BTTask_MoveToApproachPoint 자체에 모든 이동이 종료되면 성공 여부를 알 수 있는 기능을 에디터 전용으로 추가하는건 어떨까? 추후 테스트에서 이걸로 도착 여부를 판정. 테스트쪽은 설계하지 마. 지금은 BTTask_MoveToApproachPoint만.
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/BTNode/BTTask_MoveToApproachPoint.h / .cpp (수정)
FOnMoveToApproachPointFinished: 이동 종료를 알리는 델리게이트 타입. 이동한 폰, 찾던 접근 지점 종류(TargetType), 성공 여부를 넘김. 에디터 빌드에서만 선언 - QuestService.h의 DECLARE_MULTICAST_DELEGATE 형식을 따름 (신규)
OnMoveFinished: 모든 태스크 인스턴스가 공유하는 정적 델리게이트. 테스트가 여기에 구독해 도착 여부를 받음. 에디터 빌드에서만 선언 (신규)
생성자: 에디터 빌드에서만 bNotifyTaskFinished를 켜서 OnTaskFinished()가 호출되게 함 (수정, 추가)
OnTaskFinished(): 태스크가 어떤 경로로 끝나든(이동과 회전을 마친 성공, 접근 지점 없음, 이동 요청 실패, 도착 실패, 중단) 한 번 호출됨. 결과가 Succeeded일 때만 성공으로 OnMoveFinished를 브로드캐스트. 에디터 빌드에서만 오버라이드 - 엔진 UBTTaskNode::WrappedOnTaskFinished()가 호출 (신규)
ExecuteTask(), OnMoveCompleted(), TickTask(), AbortTask(), UnbindMoveCompleted(), StepFacing(): 변경 없음 (재사용: Source/Asteria/BTNode/BTTask_MoveToApproachPoint.cpp)
