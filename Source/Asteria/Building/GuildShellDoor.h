#pragma once

#include "CoreMinimal.h"
#include "GuildShellDoor.generated.h"

// PCG_GuildShell 한 변의 문 설정. 그래프 파라미터(DoorNegY/PosX/PosY/NegX) 타입으로 쓰여 디테일 패널에서 변마다 묶어 편집한다.
USTRUCT(BlueprintType)
struct FGuildShellDoor
{
	GENERATED_BODY()

	// 이 변에 문을 둘지.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door")
	bool bEnabled = false;

	// 가운데 칸에서 옮길 칸 수. 양수=건물 안에서 벽을 볼 때 오른쪽, 음수=왼쪽. 모서리에 닿으면 그래프가 멈춘다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door")
	int32 Offset = 0;
};
