#include "Game/NumGameStateBase.h"

#include "Player/NumPlayerController.h"
#include "Kismet/GameplayStatics.h"

void ANumGameStateBase::MulticastRPCBroadcastLoginMessage_Implementation(const FString& InNameString)
{
	if (HasAuthority() == false)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (IsValid(PC) == true)
		{
			ANumPlayerController* CXPC = Cast<ANumPlayerController>(PC);
			if (IsValid(CXPC) == true)
			{
				FString NotificationString = InNameString + TEXT(" has joined the game.");
				CXPC->PrintChatMessageString(NotificationString);
			}
		}
	}
}