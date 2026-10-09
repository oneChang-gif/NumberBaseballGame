#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "NumGameStateBase.generated.h"

UCLASS()
class NUMBERBASEBALLGAME_API ANumGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCBroadcastLoginMessage(const FString& InNameString = FString(TEXT("XXXXXXX")));

};
