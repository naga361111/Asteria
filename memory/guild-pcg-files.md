---
name: guild-pcg-files
description: 길드 건물 PCG 구현을 담당하는 코드·에셋·스크립트 전체 목록과 각자 역할(2026-09-29 리팩토링 후)
metadata:
  node_type: memory
  type: project
  originSessionId: 7922e10b-a6e2-4d6d-9487-1aef4f271de6
  modified: 2026-09-29T14:07:14.950Z
---

길드 건물(PCG_GuildShell) 구현을 이루는 파일들이다. 2026-09-29 리팩토링 후 기준.

**범위**: "구현된 PCG"는 Map/Building(길드 건물)과 아래 스크립트만 뜻한다. Hearthvale·Stylized_Spruce_Forest 에셋 팩과 Map/Environment/PCG_VillageNature(숲 배치)는 제외한다(2026-09-29 사용자 지시).

**원칙(사용자 지시)**: 그래프 파라미터(ShellWidth·ShellHeight·Door*)만 수정 가능하면 된다. 나머지 수치·메시는 C++ 상수로 두고, 안 쓰는 옵션·에셋은 지운다.

**C++ (Source/Asteria/Building)** — 건물 배치는 전부 C++ 노드 하나가 한다.
- `PCGGuildShellSettings.h/.cpp`: 커스텀 PCG 노드 "Guild Shell"(옛 이름 PCGGuildRoofSettings, DefaultEngine.ini ClassRedirects로 연결).
  - 속성은 덮어쓰기 핀용 ShellWidth·ShellHeight·문 8개뿐. 칸 수를 1~MaxCells(32)로 자른다.
  - 출력 핀 4개: Out(충돌 있음), NoCollision(바닥·모서리 기둥·띠보·문틀·문짝), Lights(매단 랜턴), SconceLights(벽 등).
  - 이 노드는 MCP로 추가할 수 없다. 새로 넣을 때는 명령줄 파이썬을 쓴다(핀 연결·삭제는 MCP로 됨).
- `GuildShellBuilder.h`: 치수 상수, 메시 경로 목록(GuildShell::Mesh), FGuildShellBuilder 선언. 좌표는 건물 기준(U=용마루, V=경사), 세로가 길면 90도 회전. 네 벽은 시계 방향 0=A면(V=0), 1=U=Len 박공, 2=B면, 3=U=0 박공.
- `GuildShellBuilder.cpp`: 좌표 도우미, 바닥·모서리 기둥·띠보, 외곽 벽·창 벽·문.
- `GuildShellRoof.cpp`: 지붕·박공벽. `GuildShellInterior.cpp`: 평천장·트러스·장식·조명 자리.
- `GuildShellDoor.h`: FGuildShellDoor(그래프 파라미터 DoorNegY 등의 타입).
- `GameState/Components/GuildShellService.h/.cpp`: 칸 수를 서버 권위로 복제. 태그 "GuildShellVolume" 액터의 PCG 그래프 파라미터를 바꿔 다시 만든다. 이미 같은 크기면 건너뛴다.

**에셋 (Content/Map)**
- `Building/PCG_GuildShell`: 노드 17개. 파라미터 Get 노드 10개 → GuildRoof(Guild Shell 노드) → Spawn Blocking(BlockAll)·Spawn No Collision·SpawnLights·SpawnSconceLights(PointLight 템플릿에 조명 값).
- `Building/Meshes/Roof/SM_GuildRoof_*` (9개): Slope, SlopeVerge, Eave, EaveVerge, Ridge, GableSquare(띠용 판자), GableSquare_Plaster, GableTri_Plaster, EaveFill. 판자 GableTri는 안 써서 지움.
- `Building/Meshes/Walls/SM_GuildWall_Window`, `PCG.umap`(GuildShellVolume 6×7 앞문, PP_GuildInterior 등).

**스크립트 (Tools/Blender)**: guild_roof_parts.py(지붕 부품), guild_window_wall.py(창 벽), export_hearthvale_ref.py(원본 FBX 내보내기), import_guild_parts.py(지붕·벽 FBX 임포트, 에디터 닫고 명령줄).

**검증 방법**: Saved/GuildShellSnapshot/에 6가지 크기·문 조합의 메시별 배치·조명을 JSON으로 저장해 두었다(before_=리팩토링 전). 코드를 고친 뒤 같은 조합으로 다시 찍어 비교한다.

관련: [[blender-roof-pipeline]], [[guild-interior-first]], [[pcg-mcp-regenerate-cache]], [[pcg-filter-by-index-empty-crash]], [[hearthvale-module-sizes]]
