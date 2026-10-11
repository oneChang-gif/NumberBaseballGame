#include "Game/NumGameModeBase.h"
#include "Player/NumPlayerController.h"
#include "EngineUtils.h"
#include "Player/NumPlayerState.h"
#include "Game/NumGameStateBase.h"

void ANumGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	SecretNumberString = GenerateSecretNumber();
	SubmittedGuesses.Reset();
	UE_LOG(LogTemp, Warning, TEXT("라운드 정답: %s"), *SecretNumberString);

	GetWorldTimerManager().SetTimer(TurnClockHandle, this,
		&ANumGameModeBase::UpdateTurnClock, 1.0f, true);
	StartTurnSystem();
}

void ANumGameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	ANumPlayerController* NumPC = Cast<ANumPlayerController>(NewPlayer);
	if (IsValid(NumPC) == true)
	{
		NumPC->NotificationText = FText::FromString(TEXT("Connected to the game server."));

		AllPlayerControllers.Add(NumPC);

		ANumPlayerState* NumPS = NumPC->GetPlayerState<ANumPlayerState>();
		if (IsValid(NumPS) == true)
		{
			NumPS->PlayerNameString = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());
		}

		ANumGameStateBase* NumGameStateBase = GetGameState<ANumGameStateBase>();
		if (IsValid(NumGameStateBase) && IsValid(NumPS))
		{
			NumGameStateBase->MulticastRPCBroadcastLoginMessage(NumPS->PlayerNameString);
		}
		if (HasActorBegunPlay())
		{
			StartTurnSystem();
		}
	}
}

void ANumGameModeBase::Logout(AController* Exiting)
{
	const int32 RemovedIndex = AllPlayerControllers.IndexOfByKey(Cast<ANumPlayerController>(Exiting));
	const bool bCurrentPlayerLeft = RemovedIndex != INDEX_NONE && RemovedIndex == CurrentTurnPlayerIndex;
	if (RemovedIndex != INDEX_NONE)
	{
		AllPlayerControllers.RemoveAt(RemovedIndex);
		if (RemovedIndex <= CurrentTurnPlayerIndex)
		{
			--CurrentTurnPlayerIndex;
		}
	}
	Super::Logout(Exiting);
	if (AllPlayerControllers.IsEmpty())
	{
		StopTurnSystem();
	}
	else if (bCurrentPlayerLeft)
	{
		StartNextTurn();
	}
}

FString ANumGameModeBase::GenerateSecretNumber()
{
	TArray<int32> Numbers;
	for (int32 i = 1; i <= 9; ++i)
	{
		Numbers.Add(i);
	}

	FMath::RandInit(FDateTime::Now().GetTicks());
	Numbers = Numbers.FilterByPredicate([](int32 Num) { return Num > 0; });

	FString Result;
	for (int32 i = 0; i < 3; ++i)
	{
		int32 Index = FMath::RandRange(0, Numbers.Num() - 1);
		Result.Append(FString::FromInt(Numbers[Index]));
		Numbers.RemoveAt(Index);
	}

	return Result;
}

bool ANumGameModeBase::IsGuessNumberString(const FString& InNumberString)
{
	if (InNumberString.Len() != 3)
	{
		return false;
	}

	for (TCHAR Digit : InNumberString)
	{
		if (Digit < TEXT('1') || Digit > TEXT('9'))
		{
			return false;
		}
	}

	return true;
}

FString ANumGameModeBase::JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString)
{
	int32 StrikeCount = 0, BallCount = 0;

	for (int32 i = 0; i < 3; ++i)
	{
		if (InSecretNumberString[i] == InGuessNumberString[i])
		{
			StrikeCount++;
		}
		else
		{
			FString PlayerGuessChar = FString::Printf(TEXT("%c"), InGuessNumberString[i]);
			if (InSecretNumberString.Contains(PlayerGuessChar))
			{
				BallCount++;
			}
		}
	}

	if (StrikeCount == 0 && BallCount == 0)
	{
		return TEXT("OUT");
	}

	return FString::Printf(TEXT("%dS%dB"), StrikeCount, BallCount);
}

void ANumGameModeBase::PrintChatMessageString(ANumPlayerController* InChattingPlayerController, const FString& InChatMessageString)
{
	ANumPlayerState* PlayerState = IsValid(InChattingPlayerController)
		? InChattingPlayerController->GetPlayerState<ANumPlayerState>()
		: nullptr;
	if (!IsValid(PlayerState))
	{
		return;
	}

	if (PlayerState->CurrentGuessCount >= PlayerState->MaxGuessCount)
	{
		InChattingPlayerController->ClientRPCSetChatInputEnabled(false);
		const FString NoAttemptsMessage = PlayerState->GetPlayerInfoString()
			+ TEXT(": 기회를 모두 사용했습니다.");
		for (TActorIterator<ANumPlayerController> It(GetWorld()); It; ++It)
		{
			ANumPlayerController* NumPC = *It;
			if (IsValid(NumPC))
			{
				NumPC->ClientRPCPrintChatMessageString(NoAttemptsMessage);
			}
		}
		return;
	}

	if (!CanSubmitDuringTurn(InChattingPlayerController))
	{
		InChattingPlayerController->ClientRPCPrintChatMessageString(TEXT("현재 입력할 수 있는 턴이 아닙니다."));
		return;
	}

	if (!IsGuessNumberString(InChatMessageString))
	{
		const FString InvalidMessage = PlayerState->GetPlayerInfoString()
			+ TEXT(": ") + InChatMessageString + TEXT(" -> 다시 입력하세요");
		InChattingPlayerController->ClientRPCPrintChatMessageString(InvalidMessage);
		return;
	}

	if (InChatMessageString[0] == InChatMessageString[1]
		|| InChatMessageString[0] == InChatMessageString[2]
		|| InChatMessageString[1] == InChatMessageString[2])
	{
		const FString DuplicateDigitMessage = PlayerState->GetPlayerInfoString()
			+ TEXT(": 중복되지 않은 숫자를 입력해주세요.");
		InChattingPlayerController->ClientRPCPrintChatMessageString(DuplicateDigitMessage);
		return;
	}

	if (SubmittedGuesses.Contains(InChatMessageString))
	{
		const FString RepeatedGuessMessage = PlayerState->GetPlayerInfoString()
			+ TEXT(": 이미 제출된 숫자입니다. 다시 입력하세요");
		InChattingPlayerController->ClientRPCPrintChatMessageString(RepeatedGuessMessage);
		return;
	}

	SubmittedGuesses.Add(InChatMessageString);
	bSubmittedThisTurn = true;
	IncreaseGuessCount(InChattingPlayerController);
	const FString JudgeResultString = JudgeResult(SecretNumberString, InChatMessageString);
	const int32 StrikeCount = FCString::Atoi(*JudgeResultString.Left(1));
	const FString CombinedMessageString = PlayerState->GetPlayerInfoString()
		+ TEXT(": ") + InChatMessageString + TEXT(" -> ") + JudgeResultString;
	for (TActorIterator<ANumPlayerController> It(GetWorld()); It; ++It)
	{
		ANumPlayerController* NumPC = *It;
		if (IsValid(NumPC) == true)
		{
			NumPC->ClientRPCPrintChatMessageString(CombinedMessageString);
		}
	}

	if (StrikeCount < 3 && PlayerState->CurrentGuessCount >= PlayerState->MaxGuessCount)
	{
		InChattingPlayerController->ClientRPCSetChatInputEnabled(false);
		const FString NoAttemptsMessage = PlayerState->GetPlayerInfoString()
			+ TEXT(": 기회를 모두 사용했습니다.");
		for (TActorIterator<ANumPlayerController> It(GetWorld()); It; ++It)
		{
			ANumPlayerController* NumPC = *It;
			if (IsValid(NumPC))
			{
				NumPC->ClientRPCPrintChatMessageString(NoAttemptsMessage);
			}
		}
	}

	if (StrikeCount == 3)
	{
		JudgeGame(InChattingPlayerController, StrikeCount);
		return;
	}
	FinishCurrentTurn();
}

void ANumGameModeBase::StartTurnSystem()
{
	ANumGameStateBase* NumGameState = GetGameState<ANumGameStateBase>();
	if (!IsValid(NumGameState) || NumGameState->bTurnInProgress || AllPlayerControllers.IsEmpty())
	{
		return;
	}

	StopTurnSystem();
	CurrentTurnPlayerIndex = INDEX_NONE;
	StartNextTurn();
}

void ANumGameModeBase::StartNextTurn()
{
	ANumGameStateBase* NumGameState = GetGameState<ANumGameStateBase>();
	if (!IsValid(NumGameState))
	{
		return;
	}
	NumGameState->bTurnInProgress = false;
	NumGameState->CurrentTurnPlayerState = nullptr;
	NumGameState->RemainingTurnSeconds = 0;

	for (int32 Checked = 0; Checked < AllPlayerControllers.Num(); ++Checked)
	{
		CurrentTurnPlayerIndex = (CurrentTurnPlayerIndex + 1) % AllPlayerControllers.Num();
		ANumPlayerController* NumPC = AllPlayerControllers[CurrentTurnPlayerIndex];
		ANumPlayerState* NumPS = IsValid(NumPC) ? NumPC->GetPlayerState<ANumPlayerState>() : nullptr;
		if (!IsValid(NumPS) || NumPS->CurrentGuessCount >= NumPS->MaxGuessCount)
		{
			continue;
		}

		NumGameState->CurrentTurnPlayerState = NumPS;
		NumGameState->RemainingTurnSeconds = FMath::Max(1, TurnDurationSeconds);
		NumGameState->bTurnInProgress = true;
		bSubmittedThisTurn = false;
		NumPC->ClientRPCSetChatInputEnabled(true);
		for (const auto& Other : AllPlayerControllers)
		{
			if (IsValid(Other))
			{
				Other->ClientRPCPrintChatMessageString(NumPS->PlayerNameString + TEXT("님의 턴입니다."));
			}
		}
		NumGameState->ForceNetUpdate();
		return;
	}
	StopTurnSystem();
	for (const auto& NumPC : AllPlayerControllers)
	{
		if (IsValid(NumPC) && IsValid(NumPC->GetPlayerState<ANumPlayerState>()))
		{
			JudgeGame(NumPC, 0);
			return;
		}
	}
}

void ANumGameModeBase::UpdateTurnClock()
{
	ANumGameStateBase* NumGameState = GetGameState<ANumGameStateBase>();
	if (!IsValid(NumGameState))
	{
		return;
	}

	NumGameState->ForceNetUpdate();
	if (!NumGameState->bTurnInProgress)
	{
		return;
	}
	NumGameState->RemainingTurnSeconds = FMath::Max(0, NumGameState->RemainingTurnSeconds - 1);
	NumGameState->ForceNetUpdate();
	if (NumGameState->RemainingTurnSeconds == 0)
	{
		FinishCurrentTurn();
	}
}

bool ANumGameModeBase::CanSubmitDuringTurn(ANumPlayerController* Player) const
{
	const ANumGameStateBase* NumGameState = GetGameState<ANumGameStateBase>();
	const ANumPlayerState* NumPS = IsValid(Player) ? Player->GetPlayerState<ANumPlayerState>() : nullptr;
	return IsValid(NumGameState) && IsValid(NumPS)
		&& NumGameState->bTurnInProgress && NumGameState->RemainingTurnSeconds > 0
		&& NumGameState->CurrentTurnPlayerState == NumPS && !bSubmittedThisTurn
		&& NumPS->CurrentGuessCount < NumPS->MaxGuessCount;
}

void ANumGameModeBase::FinishCurrentTurn()
{
	ANumGameStateBase* NumGameState = GetGameState<ANumGameStateBase>();
	if (!IsValid(NumGameState) || !NumGameState->bTurnInProgress)
	{
		return;
	}
	NumGameState->bTurnInProgress = false;
	ANumPlayerController* NumPC = AllPlayerControllers.IsValidIndex(CurrentTurnPlayerIndex)
		? AllPlayerControllers[CurrentTurnPlayerIndex].Get() : nullptr;
	ANumPlayerState* NumPS = IsValid(NumPC) ? NumPC->GetPlayerState<ANumPlayerState>() : nullptr;
	if (IsValid(NumPS) && !bSubmittedThisTurn && NumPS->CurrentGuessCount < NumPS->MaxGuessCount)
	{
		IncreaseGuessCount(NumPC);
		const bool bExhausted = NumPS->CurrentGuessCount >= NumPS->MaxGuessCount;
		const FString Message = NumPS->GetPlayerInfoString() + (bExhausted
			? TEXT(": 기회를 모두 사용했습니다.") : TEXT(": 시간 초과로 기회를 1회 사용했습니다."));
		for (const auto& Other : AllPlayerControllers)
		{
			if (IsValid(Other))
			{
				Other->ClientRPCPrintChatMessageString(Message);
			}
		}
		if (bExhausted)
		{
			NumPC->ClientRPCSetChatInputEnabled(false);
		}
	}
	StartNextTurn();
}

void ANumGameModeBase::StopTurnSystem()
{
	bSubmittedThisTurn = false;
	ANumGameStateBase* NumGameState = GetGameState<ANumGameStateBase>();
	if (IsValid(NumGameState))
	{
		NumGameState->bTurnInProgress = false;
		NumGameState->RemainingTurnSeconds = 0;
		NumGameState->CurrentTurnPlayerState = nullptr;
		NumGameState->ForceNetUpdate();
	}
}

void ANumGameModeBase::IncreaseGuessCount(ANumPlayerController* InChattingPlayerController)
{
	ANumPlayerState* NumPS = InChattingPlayerController->GetPlayerState<ANumPlayerState>();
	if (IsValid(NumPS) == true)
	{
		NumPS->CurrentGuessCount++;
	}
}

void ANumGameModeBase::ResetGame()
{
	StopTurnSystem();
	SecretNumberString = GenerateSecretNumber();
	SubmittedGuesses.Reset();
	UE_LOG(LogTemp, Warning, TEXT("라운드 정답: %s"), *SecretNumberString);

	for (const auto& NumPC : AllPlayerControllers)
	{
		if (!IsValid(NumPC))
		{
			continue;
		}

		ANumPlayerState* NumPS = NumPC->GetPlayerState<ANumPlayerState>();
		if (IsValid(NumPS) == true)
		{
			NumPS->CurrentGuessCount = 0;
		}
		NumPC->ClientRPCSetChatInputEnabled(true);
	}
	StartTurnSystem();
}

void ANumGameModeBase::JudgeGame(ANumPlayerController* InChattingPlayerController, int InStrikeCount)
{
	if (3 == InStrikeCount)
	{
		ANumPlayerState* NumPS = InChattingPlayerController->GetPlayerState<ANumPlayerState>();
		if (!IsValid(NumPS))
		{
			return;
		}

		for (const auto& NumPC : AllPlayerControllers)
		{
			if (IsValid(NumPC) == true)
			{
				FString CombinedMessageString = NumPS->PlayerNameString + TEXT(" has won the game.");
				NumPC->NotificationText = FText::FromString(CombinedMessageString);
			}
		}
		ResetGame();
	}
	else
	{
		bool bIsDraw = true;
		for (const auto& NumPC : AllPlayerControllers)
		{
			if (!IsValid(NumPC))
			{
				continue;
			}
			ANumPlayerState* NumPS = NumPC->GetPlayerState<ANumPlayerState>();
			if (IsValid(NumPS) == true)
			{
				if (NumPS->CurrentGuessCount < NumPS->MaxGuessCount)
				{
					bIsDraw = false;
					break;
				}
			}
		}

		if (true == bIsDraw)
		{
			for (const auto& NumPC : AllPlayerControllers)
			{
				if (IsValid(NumPC) == true)
				{
					NumPC->NotificationText = FText::FromString(TEXT("Draw..."));
				}
			}
			ResetGame();
		}
	}
}
