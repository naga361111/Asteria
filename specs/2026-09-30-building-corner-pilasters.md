요청: 모서리는 다른 기동과 동일한 에셋을 이용해서 이렇게 표현되어야 해.
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

[1단계]
PCG/BuildingLayout.h / .cpp (수정)
BuildingLayout::PilasterHalfWidth: 벽기둥 폭(59.8)의 절반 30. 모서리 벽기둥의 가장자리를 모서리에 맞추는 거리 (신규)
BuildWalls(): 칸 경계 벽기둥에 더해 벽 양 끝(Along = PilasterHalfWidth, 벽 길이 − PilasterHalfWidth)에도 같은 규칙(뒷면을 외곽선에, yaw +90)으로 Pilaster를 배치. 실내 모서리마다 두 벽의 끝 벽기둥이 맞붙어 L자가 됨 - AddOnWall() 호출 (수정)
BuildCorners(): 실내 모서리 정사각 기둥(Mesh::Corner) 배치 삭제. 바깥 모서리 기둥은 그대로 (수정)


[2단계]
Content/Map/PCG.umap (수정)
GuildShellVolume 생성 결과: 새 빌드로 PCG를 다시 생성해 저장 (수정, 추가)
