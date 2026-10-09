#include "Player/NumPlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Game/NumGameModeBase.h"
#include "NumberBaseballGame.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"
#include "UI/NumChatInput.h"

void ANumPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}

	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);

	if (IsValid(ChatInputWidgetClass) == true)
	{
		ChatInputWidgetInstance = CreateWidget<UNumChatInput>(this, ChatInputWidgetClass);
		if (IsValid(ChatInputWidgetInstance) == true)
		{
			ChatInputWidgetInstance->AddToViewport();
		}

		NotificationTextWidgetInstance = CreateWidget<UUserWidget>(this, NotificationTextWidgetClass);
		if (IsValid(NotificationTextWidgetInstance) == true)
		{
			NotificationTextWidgetInstance->AddToViewport();
		}			
	}
}

ANumPlayerController::ANumPlayerController()
{
	bReplicates = true;
}

void ANumPlayerController::SetChatMessageString(const FString& InChatMessageString)
{
	ChatMessageString = InChatMessageString;

	if (IsLocalController() == true)
	{
		// The server validates the complete input and adds the updated attempt count.
		ServerRPCPrintChatMessageString(InChatMessageString);
	}
}

void ANumPlayerController::PrintChatMessageString(const FString& InChatMessageString)
{
	//FString NetModeString = NumberBaseballGameFunctionLibrary::GetNetModeString(this);
	//FString CombinedMessageString = FString::Printf(TEXT("%s: %s"), *NetModeString, *InChatMessageString);
	//NumberBaseballGameFunctionLibrary::MyPrintString(this, CombinedMessageString, 10.f);
	
	NumberBaseballGameFunctionLibrary::MyPrintString(this, InChatMessageString, 10.f);
}

void ANumPlayerController::ClientRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	PrintChatMessageString(InChatMessageString);
}

void ANumPlayerController::ClientRPCSetChatInputEnabled_Implementation(bool bEnabled)
{
	if (IsValid(ChatInputWidgetInstance))
	{
		ChatInputWidgetInstance->SetInputEnabled(bEnabled);
	}
}

void ANumPlayerController::ServerRPCPrintChatMessageString_Implementation(const FString& InChatMessageString)
{
	//for (TActorIterator<ANumPlayerController> It(GetWorld()); It; ++It)
	//{
	//	ANumPlayerController* NumPlayerController = *It;
	//	if (IsValid(NumPlayerController) == true)
	//	{
	//		NumPlayerController->ClientRPCPrintChatMessageString(InChatMessageString);
	//	}
	//}
	AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
	if (IsValid(GM) == true)
	{
		ANumGameModeBase* CXGM = Cast<ANumGameModeBase>(GM);
		if (IsValid(CXGM) == true)
		{
			CXGM->PrintChatMessageString(this, InChatMessageString);
		}
	}
}

void ANumPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, NotificationText);
}
