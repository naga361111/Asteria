요청: 회수 대기 상태에서는 기존 색상에서 회색 계열로 바뀌도록
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
Source/Asteria/UI/Counter/WaitForAccept/CounterTileEntryWidget.h / .cpp (수정)
StateBorder: 칸 배경 Border. WBP_CounterTileEntry에 같은 이름의 Border가 있어야 함 - BindWidget (신규)
DefaultBorderColor: WBP에 설정된 원래 Brush Color를 기억하는 멤버 — 컨펌 대기 색이자 회색 계열의 원본 (신규)
NativeOnInitialized(): 기존 버튼 구독에 더해 StateBorder의 현재 Brush Color를 DefaultBorderColor로 기억 — 칸 위젯이 재사용되기 전 한 번만 - UBorder::GetBrushColor() 호출 (수정)
NativeOnListItemObjectSet(): 기존 표시에 더해 Submitted면 DefaultBorderColor, Accepted(회수 대기)면 DefaultBorderColor의 채도를 제거한 회색 계열로 칠함 — 재사용 위젯이라 Submitted일 때도 매번 원래 색으로 되돌림 - UBorder::SetBrushColor(), FLinearColor::Desaturate() 호출 (수정)
클래스 주석: WBP가 제공하는 위젯 목록에 StateBorder 추가 (수정)


[2단계]
Content/UI/Counter/WBP_CounterTileEntry.uasset (수정, 에디터 작업)
Border 이름 변경: 칸 배경 Border의 이름을 StateBorder로 변경 — 이름이 다르면 WBP 컴파일 에러 (수정)
