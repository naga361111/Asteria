요청: 퀘스트 등급별 명성과, 길드 등급 업 시 필요한 명성 데이터를 저장. 나중에 참고해서 쓸 수 있도록.
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/Data/GuildReputationData.h / .cpp (신규, Data 폴더 신규)
UGuildReputationData: 명성 표를 담는 데이터 에셋 클래스(UDataAsset 파생). 아직 사용처 없음을 주석으로 명시 (신규)
ReputationByQuestRank: 퀘스트 등급(ERank) → 정산 시 얻는 명성. 에디터에서 편집 가능 (신규)
ReputationToReachRank: 길드 등급(ERank) → 그 등급에 도달하는 데 필요한 누적 명성. 누적값이며 승급 시 초기화하지 않음을 주석으로 명시. 에디터에서 편집 가능 (신규)
UGuildReputationData(): 새 에셋이 채워진 상태로 만들어지도록 기본값 설정. ReputationByQuestRank는 F..S 순서로 1, 2, 4, 8, 16, 32, 64, ReputationToReachRank는 F..S 순서로 0, 20, 60, 160, 400, 1000, 2500 (신규, 추가)
