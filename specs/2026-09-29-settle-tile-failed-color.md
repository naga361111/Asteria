요청: WBP_CounterForSettleTileEntry 에서 StateBorder 를 잡아서 실패한 퀘스트에 대해 기본값이 붉은색 BrushColor 이도록 해줘. 선택된/호출된 일 때 색상은 건들지 마
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/UI/Counter/WaitForSettle/SettleEntryObject.h (수정)
bQuestFailed: 이 칸의 Assignment가 실패한 퀘스트인지 담는 멤버 변수. 칸 위젯이 기본 배경색을 가름 - SettleBoardWidget이 씀, SettleTileEntryWidget이 읽음 (신규)


[2단계]
Source/Asteria/UI/Counter/WaitForSettle/SettleBoardWidget.cpp (수정)
항목 생성: 칸 항목을 만들 때 Assignment의 bQuestFailed도 옮겨 담음. 필터와 나머지 값은 그대로 - USettleEntryObject::bQuestFailed 설정 (수정)

Source/Asteria/UI/Counter/WaitForSettle/SettleTileEntryWidget.h / .cpp (수정)
FailedColor: 실패한 퀘스트의 기본(정산 컨펌 대기) 배경색. 붉은색 기본값, WaitingReceiveColor처럼 WBP Class Defaults에서 조정 (신규)
NativeOnListItemObjectSet(): 수령 대기(SettleConfirmed)는 기존대로 WaitingReceiveColor. 그 외 기본 상태에서 bQuestFailed면 FailedColor, 아니면 DefaultBorderColor. 재사용 위젯이라 매 바인딩마다 다시 칠함 - UBorder::SetBrushColor() 호출 (수정)
NativeOnInitialized(), HandleSettleClicked(), DefaultBorderColor, WaitingReceiveColor: 변경 없음 (재사용: Source/Asteria/UI/Counter/WaitForSettle/SettleTileEntryWidget.cpp)
