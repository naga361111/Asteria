---
name: debugging-when-seems-no-error-in-code
description: 구현에 문제가 이상이 없는데 문제가 생길 경우 사용하는 디버깅 방법
metadata:
  node_type: memory
  type: project
  originSessionId: 7922e10b-a6e2-4d6d-9487-1aef4f271de6
  modified: 2026-09-29T10:46:07.991Z
---

코드는 분명 맞는데 런타임 동작이 이상하면 핫 리로드/Live Coding 더미를 먼저 의심한다 — stale reinstanced 클래스(`REINST_`/`SKEL_`/`HOTRELOADED_`), 구 클래스를 참조하는 Blueprint, 반영 안 된 `UPROPERTY`/디폴트 등. **에디터 재시작 + 풀 리빌드**로 재현되는지 확인한 뒤에 코드 원인을 판다.

2026-09-29 사례: 에디터를 taskkill로 끄자마자 Build.bat을 돌렸다. 그러자 `UnrealEditor-Asteria-0001.dll`이 새로 생겼고, 다시 켠 에디터는 옛 코드로 돌았다. 인스턴스 변환이 옛 값인 것을 보고 알아챘다. `Tools/Rebuild-Editor.ps1`(스킬 Asteria-Editor-Clean-Build)로 번호 붙은 DLL과 modules를 지우고 풀 빌드하니 해결됐다. 에디터를 끈 직후에는 몇 초 더 기다린 뒤 빌드하고, 빌드 뒤에는 `Binaries/Win64`에 `-000N.dll`이 생겼는지 확인한다.

두 번째로 재발했을 때의 원인은 다른 곳에 있었다. `(에디터 실행 &)`로 백그라운드에서 띄운 에디터가 하나 더 떠 있어서 DLL을 잡고 있었다(커맨드라인에 MSYS가 바꾼 `C:/Program Files/Git/Game/...` 경로가 보였다). 빌드 전에 `Get-Process UnrealEditor`로 남은 프로세스를 전부 확인하고, 에디터를 띄울 때는 `MSYS_NO_PATHCONV=1`을 붙인다.
