요청: @Content/PCG/PCG_Building.uasset 에서 바닥을 PlankFloor로 다 바꿔줘.
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

[1단계]
PCG/BuildingLayout.h (수정)
BuildingLayout::Mesh::Floor: 바닥 메시를 SM_Floor_Tiles_300에서 /Game/Hearthvale/Meshes/Floor/SM_PlanksFloor_A로 교체. 300×300·모서리 피벗·+X·-Y로 뻗는 규칙이 같아 BuildFloor() 배치는 그대로 (수정)


[2단계]
Content/Map/PCG.umap (수정)
GuildShellVolume 생성 결과: 새 빌드로 PCG를 다시 생성해 저장 (수정, 추가)
