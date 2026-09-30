---
name: blender-roof-pipeline
description: "Blender 5.2로 건물 부품을 스크립트 생성 → 명령줄로 언리얼 임포트하는 경로, 좌표·UV·재질 규칙(2026-09-29 지붕 부품에서 확인)"
metadata:
  node_type: memory
  type: reference
  originSessionId: 7922e10b-a6e2-4d6d-9487-1aef4f271de6
  modified: 2026-09-30T02:48:26.672Z
---

- Blender: `C:/Program Files/Blender Foundation/Blender 5.2/blender.exe`. MCP 연동 없이 `-b --factory-startup --python 스크립트`로 돌린다.
- 생성 스크립트는 `Tools/Blender/guild_roof_parts.py`이고, FBX를 `Intermediate/GuildRoof/`에 쓴다. 임포트는 `Tools/Blender/import_guild_parts.py`를 에디터를 닫은 상태에서 명령줄로 실행한다. 결과는 `/Game/Map/Building/Meshes/Roof/SM_GuildRoof_*`.
- 좌표: 언리얼 (x, y, z) cm = Blender (x, -y, z) m. 충돌은 `UCX_<메시이름>_NN` 오브젝트로 넣는다.
- 임포트: UE 5.8 기본 FBX 임포터(Interchange)는 AssetImportTask로 가져올 때 UCX를 무시해서 충돌이 비었다. 그래서 import 스크립트에서 `Interchange.FeatureFlags.Import.FBX false`로 기존 FBX 임포터를 쓴다. 기존 임포터는 파일의 단위 정보를 보고 m→cm로 바꾸므로, Blender 내보내기는 `apply_unit_scale=False, global_scale=1.0, FBX_SCALE_NONE`로 한다(좌표 m 그대로). 다시 임포트할 때는 옵션(import_uniform_scale 등)이 반영되지 않았다(2026-09-29).
- Hearthvale UV 밀도: 타일형 나무·돌 재질은 약 260~300cm당 반복 1회. Tavern_C 윗단 세로 판자는 MI_Trim_Wood_B_Rough 트림 시트의 V 0.502~0.996 영역이고, 높이는 143cm다.
- 너와 재질은 MI_WoodPlanks_A_Dark다. Old·C·Old_Light는 붉은 칠과 초록 얼룩이 섞여 너와가 얼룩져 보인다.
- 에디터 캡처가 하얗게 날아가면 SetCameraTransform으로 에디터 카메라를 같은 위치로 옮긴 뒤 몇 초 기다린다. 자동 노출이 카메라 위치를 따라가기 때문이다.

- 창 벽 SM_GuildWall_Window는 `Tools/Blender/guild_window_wall.py`로 만든다. Tavern_D_Window 원본(0~300, 벽면만 Tavern_C UV로 다시 입힘)과 Tavern_C 원본(300 위, 걸레받이, 중간 몰딩)을 합친 것이다. 원본 FBX는 `Intermediate/HearthvaleRef/`에 있다.
- FBX 두 개를 한 장면에 불러오면 같은 재질이 "이름.001"로 중복된다. 재질 이름으로 면을 고를 때는 `name.split(".")[0]`로 비교해야 한다. 이걸 빼먹어서 몰딩이 통째로 빠진 적이 있다.
- 기존 FBX 임포터는 다시 가져올 때 FBX에 UCX가 없으면 예전 충돌을 그대로 남긴다. 그래서 Ridge·EaveFill에 크기 잘못 가져왔던 때의 100배 충돌이 쌓였고, 건물 안 PlayerStart가 BadSize가 됐다(2026-09-29). 부품마다 `b.ucx()`를 꼭 넣는다. 임포트 스크립트는 가져오기 전에 기존 충돌을 지운다. 명령줄 실행에서는 `get_editor_subsystem`이 None이라 `EditorStaticMeshLibrary`를 쓴다.
- 명령줄 임포트는 `-script=`에 절대 경로를 줘야 한다. 상대 경로를 주면 파이썬 코드로 해석되어 NameError가 난다. 엔진 경로는 `C:\Program Files\Epic Games\UE_5.8`이다(2026-09-30).
- 창 벽을 이어 붙인 300 높이에서 D_Window 면은 y 0/0.4, Tavern_C 판자는 y 0.031/0.369에 있다. 그래서 D_Window 윗면을 지우면 3cm 틈으로 빛이 샌다. 윗면은 남긴다(2026-09-30).

관련: [[hearthvale-module-sizes]], [[visible-meshes-assets-only]]
