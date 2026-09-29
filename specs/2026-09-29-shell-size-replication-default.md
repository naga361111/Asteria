요청: 개선점 1번
빌드: "C:/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" AsteriaEditor Win64 Development "-Project=C:/Projects/Unreal/Asteria/Asteria.uproject" -WaitMutex

[1단계]
GuildShellService.h (수정)
ShellWidth: 외곽 가로 칸 수. 기본값을 8에서 0(아직 정해지지 않음)으로 바꿔, 서버 값이 무엇이든(8 포함) 기본값과 달라 클라로 복제·OnRep_ShellSize가 불리게 함. 주석에 0의 의미와 이유 기재 (수정)
ShellHeight: 외곽 세로 칸 수. ShellWidth와 같은 이유로 기본값 0 (수정)
