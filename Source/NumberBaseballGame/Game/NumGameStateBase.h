#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "NumGameStateBase.generated.h"

UCLASS()
class NUMBERBASEBALLGAME_API ANumGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Turn Practice")
	int32 RemainingTurnSeconds = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Turn Practice")
	TObjectPtr<class ANumPlayerState> CurrentTurnPlayerState = nullptr;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Turn Practice")
	bool bTurnInProgress = false;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCBroadcastLoginMessage(const FString& InNameString = FString(TEXT("XXXXXXX")));
};
