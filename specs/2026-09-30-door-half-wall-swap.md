요청: 수정 방안
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

[1단계]
PCG/BuildingLayout.h (수정)
BuildingLayout::Mesh::HalfLeft: 실내 기준 문 왼쪽(벽 진행 방향 시작 쪽) 반쪽 벽을 SM_Wall_Tavern_D_Half_rt로 교체. 밑동이 옆 일반 벽 아치 밑동과 만나고 꼭대기가 문틀에 닿음 - BuildingLayout.cpp BuildWalls()가 사용 (수정)
BuildingLayout::Mesh::HalfRight: 실내 기준 문 오른쪽 반쪽 벽을 SM_Wall_Tavern_D_Half_lt로 교체. 꼭대기가 문틀, 밑동이 옆 일반 벽과 만남 - BuildingLayout.cpp BuildWalls()가 사용 (수정)
HalfLeft / HalfRight 주석: lt/rt 이름이 바깥에서 본 기준이라 실내 기준 왼쪽에 _rt를 쓴다는 설명 (수정)


[2단계]
Content/Map/PCG.umap (수정)
GuildShellVolume 생성 결과: 새 빌드로 PCG를 다시 생성해 저장된 반쪽 벽 배치를 갱신 (수정, 추가)
