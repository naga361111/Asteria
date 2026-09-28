요청: Npc를 안전하게 제거하는 BTTaskNode 부터 만들어줘.
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/BTNode/BTTask_Despawn.h / .cpp (신규)
UBTTask_Despawn(): 노드 이름을 "Despawn"으로 지정. 실행별 상태가 없으므로 NodeMemory·노드 인스턴스화를 쓰지 않음 (신규)
ExecuteTask(): 트리를 돌리는 AI 컨트롤러의 폰을 짧은 수명(SetLifeSpan)으로 지연 제거 예약하고 InProgress를 반환 - AActor::SetLifeSpan() 호출, 폰 제거 시 엔진이 AI 컨트롤러와 BT를 함께 정리 (신규)
폰 확인: AI 컨트롤러나 폰이 없으면 Failed (신규, 추가)
서버 권위 확인: 폰이 HasAuthority가 아니면 제거하지 않고 Failed — 제거는 서버에서만, 클라는 리플리케이션으로 사라짐 (신규, 추가)
즉시 Destroy 금지: 실행 중인 BT의 주인을 태스크 안에서 바로 지우지 않도록 제거를 다음 틱 이후로 미룸 (신규, 추가)
트리 재시작 방지: Succeeded로 끝내지 않고 InProgress로 대기 — 끝내면 시퀀스가 처음(SelectQuest)부터 다시 돌아 제거 전 퀘스트를 또 집을 수 있음 (신규, 추가)
