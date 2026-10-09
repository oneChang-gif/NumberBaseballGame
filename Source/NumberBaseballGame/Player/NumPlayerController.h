#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NumPlayerController.generated.h"

class UNumChatInput;

UCLASS()
class NUMBERBASEBALLGAME_API ANumPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	ANumPlayerController();

	virtual void BeginPlay() override;

	void SetChatMessageString(const FString& InChatMessageString);

	void PrintChatMessageString(const FString& InChatMessageString);

	UFUNCTION(Client, Reliable)
	void ClientRPCPrintChatMessageString(const FString& InChatMessageString);

	UFUNCTION(Client, Reliable)
	void ClientRPCSetChatInputEnabled(bool bEnabled);

	UFUNCTION(Server, Reliable)
	void ServerRPCPrintChatMessageString(const FString& InChatMessageString);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;


protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UNumChatInput> ChatInputWidgetClass;

	UPROPERTY()
	TObjectPtr<UNumChatInput> ChatInputWidgetInstance;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> NotificationTextWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> NotificationTextWidgetInstance;

	FString ChatMessageString;

public:
	UPROPERTY(Replicated, BlueprintReadOnly)
	FText NotificationText;

};
