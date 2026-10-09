#include "AsteriaModSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IPluginManager.h"

DEFINE_LOG_CATEGORY(LogAsteriaMod);

bool UAsteriaModSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UAsteriaModSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// 서버 권위: ModEntry 스폰은 호스트만. 클라는 복제로 받는다(모드가 ModEntry의 Replicates를 켰다면).
	// ponytail: 호스트-클라 모드 목록 일치 검사 없음. 클라에 같은 모드가 없으면 복제된 ModEntry가 클라에서 풀리지 않는다.
	// 불일치가 실제 문제가 되면 접속 시 모드 목록 핸드셰이크를 추가.
	const bool bAuthority = InWorld.GetNetMode() != NM_Client;

	for (const TSharedRef<IPlugin>& Plugin : IPluginManager::Get().GetEnabledPluginsWithContent())
	{
		if (Plugin->GetType() != EPluginType::Mod)
		{
			continue;
		}

		// /<ModName>/ModEntry. 없을 수 있으므로 경고 없이 조용히 시도한다.
		const FString EntryPath = Plugin->GetMountedAssetPath() + TEXT("ModEntry.ModEntry_C");
		UClass* EntryClass = LoadClass<AActor>(nullptr, *EntryPath, nullptr, LOAD_NoWarn | LOAD_Quiet);

		UE_LOG(LogAsteriaMod, Log, TEXT("Mod: %s (ModEntry %s)"),
			*Plugin->GetName(), EntryClass ? TEXT("found") : TEXT("none"));

		if (bAuthority && EntryClass)
		{
			InWorld.SpawnActor<AActor>(EntryClass);
		}
	}
}
