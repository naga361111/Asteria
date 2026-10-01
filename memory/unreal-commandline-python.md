---
name: unreal-commandline-python
description: "MCP 끊겼을 때 UnrealEditor-Cmd -run=pythonscript로 에셋 생성 가능(에디터 실행 중에도), 출력은 unreal.log_warning으로"
metadata:
  node_type: memory
  type: reference
  originSessionId: 26447a93-2d22-4d2c-8bf9-82ccfe1b0066
  modified: 2026-10-01T13:07:50.925Z
---

Unreal MCP가 끊겨도 명령줄 파이썬으로 에셋을 만들 수 있다. 에디터가 켜져 있어도 동작했다(2026-10-01, M_BuildPreview 생성).

`"C:/Program Files/Epic Games/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "C:/Projects/Unreal/Asteria/Asteria.uproject" -run=pythonscript -script="<절대경로.py>" -unattended -nosplash -nullrhi -stdout`

- `print()`와 `unreal.log()`는 stdout grep에 안 잡혔다. 확인용 출력은 `unreal.log_warning()`으로.
- `HttpListener unable to bind to 127.0.0.1:8000` 에러는 켜진 에디터와 포트 충돌일 뿐, 무시.
- 실행 중인 에디터가 그 에셋을 열고 있으면 저장 충돌 위험 → 새 에셋 생성·읽기에만 쓰기.
