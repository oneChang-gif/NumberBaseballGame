#include "UI/NumChatInput.h"

#include "Components/EditableTextBox.h"
#include "Player/NumPlayerController.h"

void UNumChatInput::NativeConstruct()
{
	Super::NativeConstruct();

	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &ThisClass::OnChatInputTextCommitted) == false)
	{
		EditableTextBox_ChatInput->OnTextCommitted.AddDynamic(this, &ThisClass::OnChatInputTextCommitted);
	}
}

void UNumChatInput::NativeDestruct()
{
	Super::NativeDestruct();

	if (EditableTextBox_ChatInput->OnTextCommitted.IsAlreadyBound(this, &ThisClass::OnChatInputTextCommitted) == true)
	{
		EditableTextBox_ChatInput->OnTextCommitted.RemoveDynamic(this, &ThisClass::OnChatInputTextCommitted);
	}
}

void UNumChatInput::SetInputEnabled(bool bEnabled)
{
	if (IsValid(EditableTextBox_ChatInput))
	{
		EditableTextBox_ChatInput->SetIsEnabled(bEnabled);
	}
}

void UNumChatInput::OnChatInputTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod == ETextCommit::OnEnter)
	{
		APlayerController* OwningPlayerController = GetOwningPlayer();
		if (IsValid(OwningPlayerController) == true)
		{
			ANumPlayerController* OwningNumPlayerController = Cast<ANumPlayerController>(OwningPlayerController);
			if (IsValid(OwningNumPlayerController) == true)
			{
				OwningNumPlayerController->SetChatMessageString(Text.ToString());

				EditableTextBox_ChatInput->SetText(FText());
			}
		}
	}
}
