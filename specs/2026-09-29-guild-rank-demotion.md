요청: 명성이 강등 가능해야 해.
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/GameState/Components/GuildService.h / .cpp (수정)
LoseQuestReputation(): 명성 차감 뒤, 누적 명성이 현재 길드 등급의 도달 기준(ReputationData->ReputationToReachRank) 아래로 내려가면 강등. 기준을 만족하는 등급까지 연속 강등 가능, F에서 멈춤. 강등 로그를 남김. 등급과 명성은 같은 OnRep·OnGuildReputationChanged 통지를 공유하므로 통지는 기존 한 번으로 충분. 헤더 주석의 "강등 없음"을 강등 동작 설명으로 고침. 등급을 읽는 퀘스트 발행·NPC 스폰·HUD는 매번 GuildRank를 새로 읽거나 통지로 갱신되므로 수정 불필요 - UCounterService·UBTTask_SelectQuest가 호출 (수정)
LoseQuestReputation() 강등 검증: 현재 등급의 도달 기준 항목이 표에 없으면 판단할 근거가 없으므로 강등을 멈추고 로그를 남김. 명성 차감은 그대로 유지 (신규, 추가)
