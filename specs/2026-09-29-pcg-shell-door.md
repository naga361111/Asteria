요청: 한 쪽 면에 문이 배치되도록 PCG를 수정해줘.
빌드: 없음

[1단계]
Content/Map/Building/PCG_GuildShell.uasset (수정)
SelectDoorSlot: SampleWalls의 벽 점 중 앞면(외곽선 첫 변, y가 가장 작은 변)의 가운데 칸 하나를 문 칸으로 골라 나머지 벽 점과 분리 - 나머지는 SpawnWalls, 문 칸은 문 부품 노드들에 연결 (신규)
입력 검증: 앞면이 2칸 미만이면 문 칸을 고르지 않고 전부 벽으로 둠 — 모서리 기둥이 문 통로를 가리기 때문 (신규, 추가)
SpawnWalls: 문 칸을 뺀 나머지 벽 점에만 SM_Wall_Tavern_C를 놓음 (수정)
SpawnDoorCap: 문 칸 시작 쪽 180에 문 위 벽 SM_Door_Frame_B_Cap(높이 400 벽용)을 놓음 (신규)
SpawnDoorFiller: 문 칸 나머지 120에 SM_Wall_Tavern_C_Half를 길이 방향 0.8배로 놓음 — Hearthvale 원본 배치와 같은 비율 (신규)
SpawnDoorFrame: 문 통로 가운데에 문틀 SM_Door_Frame_A를 놓음 (신규)
SpawnDoorLeaves: 문틀 양쪽에 SM_Door_A_LT·SM_Door_A_RT를 실내 쪽으로 열린 상태로 놓음 — 문 여닫기 로직 없이 항상 통과 가능 (신규)
생성 확인: 8×8 기본 외곽선에서 벽 31·문 위 벽 1·채움 벽 1·문틀 1·문짝 2개 배치, 문 칸이 앞면 가운데 (추가)
통과 확인: 문틀·문 위 벽·문짝 충돌이 문 통로를 막지 않아 플레이어 캡슐이 지나갈 수 있음 (추가)
