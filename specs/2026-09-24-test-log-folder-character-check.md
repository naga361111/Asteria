요청: 기존 테스트가 로그를 쓰는 위치를 새 폴더를 만들어서 거기에 작성하도록. 지금은 찾기 힘들어. 그리고 로그를 현재는 맵 로드만 다루지만, 클라, 서버 각각에서 플레이어 Character의 로드도 검증이 필요
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/Test/Base/AsteriaNetworkComponent.h (수정)
PawnClass: 현재 맵 게임 모드의 기본 캐릭터 클래스(현재 BP_Player)를 기록하는 멤버 변수. 캐릭터 검증의 기준 (신규, 추가)
CharacterWaitStart: 캐릭터 대기를 시작한 시각을 기록하는 멤버 변수. 대기 시간 초과 판단용 (신규, 추가)
생성자: 게임 모드를 구한 뒤 PawnClass 기록. Log Loaded 다음에 AwaitCharactersLoaded(), LogCharacterLoaded() 순서로 등록 (수정)
Log(): 저장 위치를 새 폴더 Saved/TestLogs/AsteriaTest.log로 변경. 폴더가 없으면 만듦 (수정)
LogLoaded(): 변경 없음 (재사용: Source/Asteria/Test/Base/AsteriaNetworkComponent.h)
CountLoadedCharacters(): 주어진 월드의 플레이어 컨트롤러 중 PawnClass 캐릭터를 조종하고 있는 수를 셈 - AwaitCharactersLoaded(), LogCharacterLoaded()가 호출 (신규, 추가)
AwaitCharactersLoaded(): 서버 월드는 호스트와 모든 클라이언트 수만큼, 각 클라이언트 월드는 자기 캐릭터 1개가 준비될 때까지 기다림. 기본 타임아웃이 지나면 테스트를 중단하지 않고 다음 단계로 넘김 - CountLoadedCharacters() 호출, CharacterWaitStart 설정 (신규, 추가)
LogCharacterLoaded(): 서버와 클라이언트 각각 "[서버] 플레이어 캐릭터 로드 성공" 또는 "[클라이언트] 플레이어 캐릭터 로드 실패" 형식으로 한 줄 기록. 실패면 기대 수와 실제 수를 테스트 오류로 보고 - CountLoadedCharacters(), Log() 호출 (신규)
