요청: 개선점 2번
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

[1단계]
GuildShellBuilder.h (수정)
FGuildShellMeshPoints: namespace GuildShell 안으로 이동 (수정)
FGuildShellBuilder: namespace GuildShell 안으로 이동 - PCGGuildShellSettings.cpp에서 GuildShell::FGuildShellBuilder로 사용됨 (수정)

GuildShellBuilder.cpp (수정)
using namespace GuildShell: 파일 범위 using 지시문 삭제 (수정)
파일 본문(익명 namespace 상수·DoorCenter, FGuildShellBuilder 멤버 정의 전체): namespace GuildShell { }로 감쌈 (수정)

GuildShellRoof.cpp (수정)
using namespace GuildShell: 파일 범위 using 지시문 삭제 (수정)
파일 본문(익명 namespace 상수, BuildRoof·BuildGables 정의): namespace GuildShell { }로 감쌈 (수정)

GuildShellInterior.cpp (수정)
using namespace GuildShell: 파일 범위 using 지시문 삭제 (수정)
파일 본문(익명 namespace 상수, BuildCeiling·BuildDecor 정의): namespace GuildShell { }로 감쌈 (수정)

PCGGuildShellSettings.cpp (수정)
ExecuteInternal(): 빌더 타입을 GuildShell::FGuildShellBuilder로 한정 - GuildShellBuilder.h 참조 (수정)
