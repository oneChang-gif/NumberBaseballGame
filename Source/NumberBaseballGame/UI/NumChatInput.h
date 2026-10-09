#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"

#include "NumChatInput.generated.h"


class UEditableTextBox;

UCLASS()
class NUMBERBASEBALLGAME_API UNumChatInput : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	void SetInputEnabled(bool bEnabled);

protected:
	UFUNCTION()
	void OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);


public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> EditableTextBox_ChatInput;
};
