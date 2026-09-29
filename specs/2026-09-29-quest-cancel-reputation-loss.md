요청: 내가 의도한 루트에서 잡은 퀘스트가 있을 경우, 본래 해당 퀘스트가 정상적으로 완료될 때 적립할 명성 포인트를 해당 루트로 취소되면 길드 명성에서 빼줘. 기다리다 시간 초과의 경우 카운터에 한 번 오게되는데, 이 때, 해당 작업을 수행. 퀘스트 실패의 경우도 정산 단계에 해당 내용을 실행. 애초에 퀘스트를 못잡는 경우는 Npc의 등급을 퀘스트의 등급으로 판단하고 그거만큼 길드 명성 감소/
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/GameState/Components/GuildService.h / .cpp (수정)
LoseQuestReputation(): 인자로 받은 퀘스트 등급의 명성(ReputationData->ReputationByQuestRank)만큼 누적 명성을 줄임. 0 아래로 내려가지 않음. 등급 강등은 하지 않음(별도 TodoList 항목). 로그를 남기고 서버에서 직접 OnGuildReputationChanged 통지 - UCounterService::LoseQuestAssignmentReputation()·UCounterService::ReceiveSettleQuestAssignment()·UBTTask_SelectQuest::ExecuteTask()가 호출 (신규)
LoseQuestReputation() 권위·입력 검증: 권위 없음, 명성 표 없음, 표에 해당 등급 항목 없음이면 아무것도 바꾸지 않고 false (신규, 추가)


[2단계]
Source/Asteria/GameState/Components/CounterService.h / .cpp (수정)
LoseQuestAssignmentReputation(): Assignment의 퀘스트 등급만큼 길드 명성을 차감. 대기 시간 초과로 창구에서 떠나는 NPC가 들고 있던 퀘스트를 취소할 때 씀 - UQuestService::FindQuestAssignment(), UGuildService::LoseQuestReputation() 호출, UBTTask_WaitForConfirmQuest::TickTask()·UBTTask_WaitForSettleConfirm::TickTask()가 호출 (신규)
LoseQuestAssignmentReputation() 입력 검증: 서비스 없음, Assignment 없음, 그 퀘스트가 QuestPull에 없음이면 차감하지 않고 false (신규, 추가)
ReceiveSettleQuestAssignment(): 실패한 퀘스트(bQuestFailed)는 수령 전에 퀘스트 등급을 값으로 복사해 두고, 수령이 성공한 뒤 그 등급만큼 길드 명성을 차감. 수령 후엔 퀘스트가 보드로 돌아가거나 지워질 수 있어 등급은 수령 전에 확보. 퀘스트가 없으면 차감만 건너뛰고(로그) 수령은 막지 않음. 성공한 퀘스트는 기존대로 - UQuestService::ReceiveSettleQuestAssignment(), UGuildService::LoseQuestReputation() 호출 (수정)

Source/Asteria/BTNode/BTTask_SelectQuest.cpp (수정)
ExecuteTask(): 집을 퀘스트가 없어(FindAvailableQuestId가 INDEX_NONE) 실패할 때만 Npc->NpcRnk를 퀘스트 등급으로 보고 그만큼 길드 명성을 차감. 다른 NPC가 먼저 집어 AssignQuest가 거절된 경우는 의도한 루트가 아니므로 차감하지 않음 - UGuildService::LoseQuestReputation() 호출 (수정)


[3단계]
Source/Asteria/BTNode/BTTask_WaitForConfirmQuest.cpp (수정)
TickTask(): 컨펌 대기 마감이 지나 Failed로 끝나기 직전에 기다리던 Assignment의 퀘스트 등급만큼 길드 명성을 차감. 이 시점엔 Assignment가 아직 Submitted로 남아 있음(반환은 뒤따르는 떠나기 가지가 함) - UCounterService::LoseQuestAssignmentReputation() 호출 (수정)

Source/Asteria/BTNode/BTTask_WaitForSettleConfirm.cpp (수정)
TickTask(): 정산 컨펌 대기 마감이 지나 Failed로 끝나기 직전에 기다리던 Assignment의 퀘스트 등급만큼 길드 명성을 차감. 수행 성공·실패 어느 쪽이든 차감은 여기서 한 번뿐 — 수령(ReceiveSettle)에 도달하지 않으므로 이중 차감 없음 - UCounterService::LoseQuestAssignmentReputation() 호출 (수정)
