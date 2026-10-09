#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NumPlayerState.generated.h"

UCLASS()
class NUMBERBASEBALLGAME_API ANumPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	ANumPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	FString GetPlayerInfoString();

public:
	UPROPERTY(Replicated)
	FString PlayerNameString;

	UPROPERTY(Replicated)
	int32 CurrentGuessCount;

	UPROPERTY(Replicated)
	int32 MaxGuessCount;

};
