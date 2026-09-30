요청: Hearthvale의 Tabletop 에셋 세트를 이용해서 간단한 카운터?를 만드는 PCG를  구현해줘. PCG를 사용하는 이유는 동일한 패턴을 유지하면서, 작은 카운터에서, 큰 카운터로 변할 수 있어야 하기 때문. 에셋은 Content의 PCG에, 코드는 Source/Asteria/PCG 에. 필요 시 얘 전용 폴더를 루트에 만들어도 돼. Tabletop 에셋만 이용. (수정: 건물 PCG와 별개의 PCG 그래프·노드로 만든다. 건물 노드와 겹치는 헬퍼만 공용 파일로 분리한다.)
빌드: pwsh -File C:\Projects\Unreal\Asteria\Tools\Rebuild-Editor.ps1

[1단계]
PCG/PCGMeshPoints.h / .cpp (신규)
PCGMeshPoints::MeshAttribute: 점의 메시 경로 속성 이름 "Mesh". Static Mesh Spawner(By Attribute)가 읽음 (신규, 알고리즘 출처: PCG/PCGBuildingSettings.cpp BuildingMeshAttribute)
PCGMeshPoints::GetVolumeOrigin(): 실행 소스(볼륨)의 로컬 경계 최소 모서리를 원점으로, 볼륨 위치·회전은 따르고 스케일은 뺀 트랜스폼. 실행 소스가 없으면 항등 - PCGBuildingSettings.cpp, PCGCounterSettings.cpp가 호출 (신규, 알고리즘 출처: PCG/PCGBuildingSettings.cpp GetBuildingOrigin)
PCGMeshPoints::MakePointData(): 트랜스폼·메시 경로 배열로 점 데이터를 만들고 점마다 MeshAttribute를 담 - PCGBuildingSettings.cpp, PCGCounterSettings.cpp가 호출 (신규, 알고리즘 출처: PCG/PCGBuildingSettings.cpp MakeBuildingPointData)


[2단계]
PCG/PCGBuildingSettings.cpp (수정)
BuildingMeshAttribute / MakeBuildingPointData() / GetBuildingOrigin(): 익명 네임스페이스에서 삭제 (수정)
FPCGBuildingElement::ExecuteInternal(): PCGMeshPoints::GetVolumeOrigin(), PCGMeshPoints::MakePointData() 호출로 교체. 동작 불변 (수정)

PCG/PCGCounterSettings.h / .cpp (신규)
CounterLayout::MaxMidCount: 가운데 판 개수 상한 10 (신규)
CounterLayout::Mid: 가운데 판 한 개 길이 300 (신규)
CounterLayout::EndWidth: 끝 판(Lt_150/Rt_150)의 공칭 폭 150. 카운터 몸통 시작 X (신규)
CounterLayout::EndDepth: 끝 판이 -Y로 꺾여 내려가는 깊이 170. 카운터 몸통 Y (신규)
CounterLayout::Mesh::Left / Mid / Right: SM_Tabletop_Lt_150(피벗 오른쪽 끝, -X로 뻗고 -Y로 꺾임) / SM_Tabletop_Mid_300(피벗 왼쪽 끝, +X로 300, 깊이 -Y 107) / SM_Tabletop_Rt_150(피벗 왼쪽 끝, +X로 뻗고 -Y로 꺾임). 로컬 +Y 면이 손님 쪽, 높이 132 (신규)
UPCGCounterSettings::MidCount: 가운데 판 개수, PCG_Overridable, 0~MaxMidCount. 그래프 파라미터 MidCount를 받음. 0이면 끝 판 둘만 붙은 가장 작은 카운터 (신규)
GetDefaultNodeName() / GetDefaultNodeTitle() / GetNodeTooltipText() / GetType(): 에디터 노드 이름 "Counter", 제목 "Counter", 툴팁, Spatial (신규)
InputPinProperties(): 입력 핀 없음 (신규)
OutputPinProperties(): 출력 핀 Out 하나(점, Mesh 속성 포함) (신규)
CreateElement(): FPCGCounterElement 생성 (신규)
FPCGCounterElement::IsCacheable(): 항상 false. 결과가 볼륨 트랜스폼에 따라 달라지는데 입력 핀이 없어 캐시 키에 안 잡힘 (신규, 추가)
FPCGCounterElement::ExecuteInternal(): 로컬 (EndWidth, EndDepth)에서 시작해 Left 하나, Mid를 300 간격으로 MidCount개, 마지막 Mid 끝에 Right 하나를 모두 yaw 0으로 놓고, 모든 점에 볼륨 원점을 곱해 Out으로 출력. 카운터가 볼륨 로컬 경계 최소 모서리부터 X 0~(300+MidCount×300), Y 0~EndDepth를 차지함 - PCGMeshPoints::GetVolumeOrigin(), PCGMeshPoints::MakePointData() 호출 (신규)
MidCount 범위 검증: 그래프 파라미터(외부 입력)를 0~MaxMidCount로 자름 (신규, 추가)


[3단계]
Content/PCG/PCG_Counter.uasset (신규)
그래프 파라미터 MidCount: int32, 기본 2 (신규)
Get Graph Parameter: MidCount를 읽음 - Counter 노드의 MidCount 핀에 연결 (신규)
Counter 노드: UPCGCounterSettings - Out을 Static Mesh Spawner에 연결 (신규)
Static Mesh Spawner: By Attribute(Mesh)로 메시 배치, 충돌 있음 - Output 노드에 연결 (신규)

Content/Map/PCG.umap (수정)
CounterVolume: 건물 볼륨과 겹치지 않는 자리에 PCG 볼륨 하나를 놓고 그래프 /Game/PCG/PCG_Counter 지정, 인스턴스 파라미터 MidCount 2 (신규, 추가)
