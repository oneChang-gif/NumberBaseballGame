#include "UI/NumTimeWidget.h"
#include "Components/TextBlock.h"
#include "Game/NumGameStateBase.h"
#include "Player/NumPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

void UNumTimeWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshTimeText();
}

void UNumTimeWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshTimeText();
}

void UNumTimeWidget::RefreshTimeText()
{
	const ANumGameStateBase* NumGameState = GetWorld() ? GetWorld()->GetGameState<ANumGameStateBase>() : nullptr;
	const APlayerController* OwningPlayerController = GetOwningPlayer();
	const ANumPlayerState* NumPS = IsValid(OwningPlayerController)
		? OwningPlayerController->GetPlayerState<ANumPlayerState>()
		: nullptr;
	if (!IsValid(NumGameState))
	{
		if (RemainingTurnTimeText)
		{
			RemainingTurnTimeText->SetText(FText::FromString(TEXT("남은 시간 --:--")));
		}
		if (TurnCheckText)
		{
			TurnCheckText->SetText(FText::FromString(TEXT("턴 정보 대기 중")));
		}
		return;
	}
	const int32 Remaining = FMath::Max(0, NumGameState->RemainingTurnSeconds);
	if (RemainingTurnTimeText)
	{
		RemainingTurnTimeText->SetText(FText::FromString(FString::Printf(
			TEXT("남은 시간 %02d:%02d"), Remaining / 60, Remaining % 60)));
	}
	if (!TurnCheckText)
	{
		return;
	}
	if (!IsValid(NumPS))
	{
		TurnCheckText->SetText(FText::FromString(TEXT("턴 정보 대기 중")));
		return;
	}
	FString TurnLabel = TEXT("턴 대기 중");
	if (NumGameState->bTurnInProgress && IsValid(NumGameState->CurrentTurnPlayerState))
	{
		TurnLabel = NumGameState->CurrentTurnPlayerState == NumPS ? TEXT("내 턴")
			: NumGameState->CurrentTurnPlayerState->PlayerNameString + TEXT("님의 턴");
	}
	const int32 RemainingTurns = FMath::Max(0, NumPS->MaxGuessCount - NumPS->CurrentGuessCount);
	TurnCheckText->SetText(FText::FromString(FString::Printf(
		TEXT("%s / 내 남은 기회 %d회"), *TurnLabel, RemainingTurns)));
}
