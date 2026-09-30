요청: 제시한 레퍼런스 이미지처럼, 벽마다 /Game/Hearthvale/Meshes/Walls/SM_Wall_House_A_Column.SM_Wall_House_A_Column 를 배치해. (해당 에셋은 이외에도 _B, _C가 존재. 사이즈를 보고 적절히 활용)
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

[1단계]
PCG/BuildingLayout.h / .cpp (수정)
BuildingLayout::WallInnerFace: _D 벽 실내 면이 외곽선에서 안쪽으로 떨어진 거리 6.4(일반 4.5·창 6.4 중 큰 값). 벽기둥이 이 면에 붙음 (신규)
BuildingLayout::PilasterHalfDepth: 벽기둥 두께(25.2)의 절반 12.6. 실내 면에서 벽기둥 중심까지 거리 (신규)
BuildingLayout::Mesh::Pilaster: SM_Wall_House_A_Column(폭 60 × 두께 25 × 높이 305, 아래 가운데 피벗, 폭이 로컬 Y). 벽면에 붙는 납작한 벽기둥 (신규)
BuildWalls(): 벽마다 칸 경계(양 끝 모서리 제외, 1~Cells−1)에 Pilaster를 실내 면에 붙여 폭이 벽을 따라가게 배치. 문·창 옆 경계도 포함 - AddOnWall() 호출 (수정)
BuildCorners(): 네 실내 모서리에 Mesh::Corner(SM_Wall_House_A_Column_B, 40×40×300)를 두 벽 실내 면에 붙여 배치. 벽기둥을 모서리에 두면 옆 벽에 반쯤 묻혀 정사각 기둥으로 대신함. 바깥 모서리 기둥은 그대로 - Add() 호출 (수정)
AddOnWall(): 실내 쪽 거리(Lateral)와 yaw 오프셋 인자를 받음 - Add() 호출 (수정)


[2단계]
Content/Map/PCG.umap (수정)
GuildShellVolume 생성 결과: 새 빌드로 PCG를 다시 생성해 저장 (수정, 추가)
