#include "Player/NumPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Game/NumGameModeBase.h"
#include "NumberBaseballGame.h"
#include "Net/UnrealNetwork.h"
#include "UI/NumChatInput.h"
#include "UI/NumTimeWidget.h"

void ANumPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}

	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);

	if (!TimeWidgetClass)
	{
		TimeWidgetClass = LoadClass<UNumTimeWidget>(nullptr,
			TEXT("/Game/NumGame/Blueprint/UI/WBP_Time.WBP_Time_C"));
	}
	if (TimeWidgetClass)
	{
		TimeWidgetInstance = CreateWidget<UNumTimeWidget>(this, TimeWidgetClass);
		if (IsValid(TimeWidgetInstance))
		{
			TimeWidgetInstance->AddToViewport();
		}
	}

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
		ServerRPCPrintChatMessageString(InChatMessageString);
	}
}

void ANumPlayerController::PrintChatMessageString(const FString& InChatMessageString)
{

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
	AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
	if (IsValid(GM) == true)
	{
		ANumGameModeBase* NumGM = Cast<ANumGameModeBase>(GM);
		if (IsValid(NumGM) == true)
		{
			NumGM->PrintChatMessageString(this, InChatMessageString);
		}
	}
}

void ANumPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, NotificationText);
}
