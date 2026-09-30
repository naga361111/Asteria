요청: 이와 같이 벽과 착 달라붙어서 겹쳐있어야 해.
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

[1단계]
PCG/BuildingLayout.h / .cpp (수정)
BuildingLayout::WallInnerFace: 기둥을 실내 면 앞에 세우던 기준이라 삭제. 일반 벽 실내 면(4.5)과 기둥 뒷면(6.4) 사이에 1.9 틈이 생기던 원인 (수정)
BuildWalls(): 벽기둥 뒷면을 외곽선(벽 두께 안)에 맞춰 벽에 4.5~6.4 묻히고 나머지가 실내로 튀어나오게 배치(중심 = 외곽선에서 PilasterHalfDepth) - AddOnWall() 호출 (수정)
BuildCorners(): 실내 모서리 기둥도 같은 규칙으로 두 벽 외곽선에 뒷면을 맞춰 벽에 묻히게 배치(중심 = 두 외곽선에서 CornerInset) - Add() 호출 (수정, 추가)


[2단계]
Content/Map/PCG.umap (수정)
GuildShellVolume 생성 결과: 새 빌드로 PCG를 다시 생성해 저장 (수정, 추가)
