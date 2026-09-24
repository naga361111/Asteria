요청: 테스트 파일은 좀 다른 구조를 형성하려 해. 먼저 서버와 클라를 띄우고 현재 Map을 주입하는 모든 테스트에 공통적으로 적용되는 기능을 우선적으로 구현해두려 해. 향후 테스트는 이를 기반으로 실행되는거야. 얘는 Source/Test/Base 폴더에 위치해야 해. 로깅은 클라이언트와 서버가 제대로 로드되었는지 각각 한 줄이 전부.
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/Asteria.Build.cs (수정)
PrivateDependencyModuleNames: 에디터 빌드 블록에 EngineSettings 추가. 프로젝트 기본 게임 모드 조회에 필요. 같은 블록의 CQTest, UnrealEd는 그대로 사용 (수정, 추가)


[2단계]
Source/Asteria/Test/Base/AsteriaNetworkComponent.h (신규)
컴파일 조건: 파일 전체를 에디터 빌드이면서 ENABLE_PIE_NETWORK_TEST일 때만 컴파일. 게임 빌드에는 CQTest가 없기 때문 (신규, 추가)
FAsteriaNetworkComponent: 서버와 클라이언트를 띄우고 현재 맵을 싣는 공통 기반. 테스트 클래스가 멤버로 두고 ThenServer, ThenClient 등으로 단계를 이어 붙임 - FPIENetworkComponent 상속 (재사용: Engine/Source/Developer/CQTest/Public/Components/PIENetworkComponent.h) (신규)
MapName: 테스트 시작 시점에 에디터에 열려 있는 맵의 이름을 기록하는 멤버 변수 (신규)
생성자: 부모 생성자를 초기화 모드로 불러 엔진 기본 준비 단계(빈 새 맵 생성)가 등록되지 않게 함. 초기화 중이 아닐 때만 MapName 기록, 현재 맵의 게임 모드(World Settings, 없으면 프로젝트 기본 게임 모드)로 리슨 서버와 클라이언트 1개를 구성, PIE 중지, PIE 시작, 월드 확보 대기, 패킷 설정, 클라이언트 접속, 클라이언트 준비 대기, LogLoaded() 순서로 등록하고 테스트 종료 시 에디터 상태 복원을 등록 - FNetworkComponentBuilder와 부모의 StopPie, StartPie, SetWorlds, SetPacketSettings, ConnectClientsToServer, AwaitClientsReady, RestoreState 호출 (재사용: Engine/Source/Developer/CQTest/Private/Components/PIENetworkComponent.cpp) (신규)
LogLoaded(): 서버 월드와 클라이언트 월드의 맵 이름(PIE 접두어 제외)이 MapName과 같은지 확인하고 "[서버] <맵> 맵 로드 성공" 또는 "[클라이언트] <맵> 맵 로드 실패" 형식으로 각각 한 줄 기록. 다르면 테스트 오류로 보고 - LogTemp 사용 (신규)


[3단계]
Source/Asteria/Test/Base/AsteriaNetworkSmokeTest.cpp (신규)
AsteriaNetworkSmoke: FAsteriaNetworkComponent를 멤버로 두고 빈 테스트 메서드 하나만 가진 네트워크 테스트. 테스트 경로 Asteria.Base.Network. 공통 기반을 실제로 컴파일하고 실행해 로그를 확인하는 용도. 에디터 빌드에서만 컴파일 - NETWORK_TEST_CLASS 사용 (재사용: Engine/Source/Developer/CQTest/Public/Components/PIENetworkComponent.h) (신규, 추가)
