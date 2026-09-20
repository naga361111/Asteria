---
name: debugging when seems no error in code
description: 구현에 문제가 이상이 없는데 문제가 생길 경우 사용하는 디버깅 방법
metadata:
  type: project
---

코드는 분명 맞는데 런타임 동작이 이상하면 핫 리로드/Live Coding 더미를 먼저 의심한다 — stale reinstanced 클래스(`REINST_`/`SKEL_`/`HOTRELOADED_`), 구 클래스를 참조하는 Blueprint, 반영 안 된 `UPROPERTY`/디폴트 등. **에디터 재시작 + 풀 리빌드**로 재현되는지 확인한 뒤에 코드 원인을 판다.