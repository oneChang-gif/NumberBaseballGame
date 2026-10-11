#include "Game/NumGameStateBase.h"

#include "Player/NumPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/NumPlayerState.h"

void ANumGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANumGameStateBase, RemainingTurnSeconds);
	DOREPLIFETIME(ANumGameStateBase, CurrentTurnPlayerState);
	DOREPLIFETIME(ANumGameStateBase, bTurnInProgress);
}

void ANumGameStateBase::MulticastRPCBroadcastLoginMessage_Implementation(const FString& InNameString)
{
	if (HasAuthority() == false)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (IsValid(PC) == true)
		{
			ANumPlayerController* NumPC = Cast<ANumPlayerController>(PC);
			if (IsValid(NumPC) == true)
			{
				FString NotificationString = InNameString + TEXT(" has joined the game.");
				NumPC->PrintChatMessageString(NotificationString);
			}
		}
	}
}
