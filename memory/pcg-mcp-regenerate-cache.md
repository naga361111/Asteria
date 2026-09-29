---
name: pcg-mcp-regenerate-cache
description: MCP로 PCG 노드 설정(스포너 메시 등)을 바꾼 뒤 ExecuteGraphInstance만 하면 옛 결과가 그대로 나옴 — 인스턴스 파라미터를 한 번 바꿨다 되돌려야 재생성. Git Bash에선 /Game 경로가 변환되니 MSYS_NO_PATHCONV=1
metadata:
  node_type: memory
  type: project
  originSessionId: 7922e10b-a6e2-4d6d-9487-1aef4f271de6
  modified: 2026-09-29T10:23:58.016Z
---

2026-09-29 PCG_GuildShell 천장 작업 중 발견.
- Static Mesh Spawner의 메시를 set_properties로 바꾸고 저장 → ExecuteGraphInstance를 불러도 생성된 ISM이 옛 메시 그대로였다. GuildShellVolume의 ShellWidth를 +1 했다가 되돌리며 두 번 실행하니 반영됐다(스크래치 regen.py).
- Git Bash에서 인자로 넘긴 `/Game/...`는 `C:/Program Files/Git/Game/...`로 바뀐다. set_properties는 true를 돌려주지만 메시가 None으로 들어간다. `MSYS_NO_PATHCONV=1`로 막는다.

- MCP PCGToolset은 `/Script/PCG` 패키지 노드만 추가할 수 있다(PCGToolsetLibraryCore가 그 패키지만 등록). 프로젝트 C++ PCG 노드(예: `UPCGGuildRoofSettings`)는 에디터를 닫고 명령줄 파이썬에서 넣는다. `graph.add_node_of_type(cls)`, `add_edge`, `remove_node`를 쓴다. 노드 위치는 이후 MCP의 RepositionNode로 옮긴다.
- 에디터를 강제 종료(taskkill /F)하면, 다음 실행 때 "패키지 복구" 모달이 떠서 MCP가 응답하지 않는다. 종료 전에 더러운 패키지가 없는지 is_dirty로 확인하고, 떴다면 `Saved/Autosaves/PackageRestoreData.json`을 치운 뒤 다시 실행한다.

**Why:** 둘 다 오류 없이 "성공"을 돌려줘서 반영 안 된 걸 캡처로만 알아챘다.
**How to apply:** 노드 설정을 바꾼 뒤엔 컴포넌트 메시/개수로 반영을 확인하고, 안 바뀌었으면 파라미터를 흔들어 재생성한다. 관련: [[pcg-filter-by-index-empty-crash]]
