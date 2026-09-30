요청: 아래쪽 기둥을 위에도 한 칸 쌓아서 모서리 Plank 사이에 공백이 없도록. Plank와 맞는 사이즈가 있으면 그거 쓰고. 없으면 기존꺼 사용
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

[1단계]
PCG/BuildingLayout.h / .cpp (수정)
BuildCorners(): 바깥 모서리마다 기존 기둥(Mesh::Corner, 40×40×300) 위 WallHeight에 같은 기둥을 원본 크기 그대로 하나 더 쌓아 판자 줄 모서리 틈을 채움(윗단은 판자 위로 튀어나옴). 160 높이 나무 기둥 에셋이 없어 기존 에셋을 씀 - Add() 호출 (수정)


[2단계]
Content/Map/PCG.umap (수정)
GuildShellVolume 생성 결과: 새 빌드로 PCG를 다시 생성해 저장 (수정, 추가)
