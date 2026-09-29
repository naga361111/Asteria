---
name: blender-roof-pipeline
description: "Blender 5.2로 건물 부품을 스크립트 생성 → 명령줄로 언리얼 임포트하는 경로, 좌표·UV·재질 규칙(2026-09-29 지붕 부품에서 확인)"
metadata:
  node_type: memory
  type: reference
  originSessionId: 7922e10b-a6e2-4d6d-9487-1aef4f271de6
  modified: 2026-09-29T09:49:38.892Z
---

- Blender: `C:/Program Files/Blender Foundation/Blender 5.2/blender.exe`. MCP 연동 없이 `-b --factory-startup --python 스크립트`로 돌린다.
- 생성 스크립트는 `Tools/Blender/guild_roof_parts.py`이고, FBX를 `Intermediate/GuildRoof/`에 쓴다. 임포트는 `Tools/Blender/import_guild_roof.py`를 에디터를 닫은 상태에서 명령줄로 실행한다. 결과는 `/Game/Map/Building/Meshes/Roof/SM_GuildRoof_*`.
- 좌표: 언리얼 (x, y, z) cm = Blender (x, -y, z) m. FBX 기본 축(-Z forward, Y up)과 apply_unit_scale로 그대로 맞는다. 충돌은 `UCX_<메시이름>_NN` 오브젝트로 넣는다.
- Hearthvale UV 밀도: 타일형 나무·돌 재질은 약 260~300cm당 반복 1회. Tavern_C 윗단 세로 판자는 MI_Trim_Wood_B_Rough 트림 시트의 V 0.502~0.996 영역이고, 높이는 143cm다.
- 너와 재질은 MI_WoodPlanks_A_Dark다. Old·C·Old_Light는 붉은 칠과 초록 얼룩이 섞여 너와가 얼룩져 보인다.
- 에디터 캡처가 하얗게 날아가면 SetCameraTransform으로 에디터 카메라를 같은 위치로 옮긴 뒤 몇 초 기다린다. 자동 노출이 카메라 위치를 따라가기 때문이다.

관련: [[hearthvale-module-sizes]], [[visible-meshes-assets-only]]
