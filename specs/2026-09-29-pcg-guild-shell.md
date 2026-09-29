요청: 구체적으로 설계 (Hearthvale로 바닥·벽만 구성하는 건물 외곽 PCG, /Game/Map/PCG 레벨)
빌드: 없음

[1단계]
Content/Map/Building/PCG_GuildShell.uasset (신규)
GetOutline: 월드에서 태그 `GuildShell`이 붙은 액터의 스플라인(건물 외곽선)을 읽는 Get Spline Data 노드 - GetCorners에 연결 (신규)
GetCorners: 외곽선 꼭짓점을 점으로 뽑는 Get Spline Control Points 노드 - SnapCorners에 연결 (신규)
SnapCorners: 꼭짓점 X·Y를 300 격자에 반올림해 벽 구간 길이를 300의 배수로 맞춤 - DedupeCorners에 연결 (신규, 추가)
DedupeCorners: 반올림 후 같은 위치로 겹친 꼭짓점을 제거해 길이 0 구간을 막음 - BuildOutline, SpawnCorners에 연결 (신규, 추가)
BuildOutline: 정리된 꼭짓점으로 닫힌 직선 스플라인을 만드는 Create Spline 노드 - ForceWinding에 연결 (신규)
ForceWinding: 스플라인 진행 방향을 한쪽으로 고정해 벽의 실내 면이 외곽선 위에 오고 두께(-Y)가 바깥을 향하게 하는 Spline Direction 노드 - MakeFloorSurface, SampleWalls에 연결 (신규, 추가)
MakeFloorSurface: 외곽선 안쪽 면을 만드는 Create Surface From Spline 노드 - SampleFloor에 연결 (신규)
SampleFloor: 안쪽 면을 300 칸 중심 점으로 채우는 Surface Sampler 노드 - AlignFloor에 연결 (신규)
AlignFloor: 바닥 메시의 모서리 피벗(+X, -Y로 뻗음)에 맞게 점을 칸 모서리로 옮기는 Transform Points 노드 - SpawnFloor에 연결 (신규)
SpawnFloor: 점마다 SM_Floor_Tiles_300을 놓는 Static Mesh Spawner 노드 (신규)
SampleWalls: 외곽선을 따라 300 간격으로 점을 찍고 선 방향으로 회전시키는 Spline Sampler 노드, 닫힌 선 끝점 중복 없음 - SpawnWalls에 연결 (신규)
SpawnWalls: 점마다 SM_Wall_Tavern_C(높이 400)를 놓는 Static Mesh Spawner 노드 (신규)
SpawnCorners: 꼭짓점마다 SM_Column_400을 놓아 벽 모서리 이음새를 가리는 Static Mesh Spawner 노드 (신규)


[2단계]
Content/Map/PCG.umap (수정)
GuildShellOutline: 태그 `GuildShell`, 닫힌 직선 스플라인, 2400×2400(8×8칸) 직사각형 기본 외곽선 액터 - GetOutline이 읽음 (신규)
GuildShellVolume: PCG_GuildShell 그래프를 쓰는 PCG Volume, 외곽선 전체를 덮는 크기 (신규)
생성 확인: 그래프 실행 오류 없음, 바닥 64·벽 32·기둥 4개 배치, 벽 실내 면이 외곽선 위에 옴 (추가)
