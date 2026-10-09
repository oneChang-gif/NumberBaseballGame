#include "Game/NumGameModeBase.h"
#include "Player/NumPlayerController.h"
#include "EngineUtils.h"
#include "Player/NumPlayerState.h"
#include "NumGameStateBase.h"

void ANumGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	SecretNumberString = GenerateSecretNumber();
	SubmittedGuesses.Reset();
	UE_LOG(LogTemp, Warning, TEXT("라운드 정답: %s"), *SecretNumberString);
}

void ANumGameModeBase::OnPostLogin(AController* NewPlayer)
{
    Super::OnPostLogin(NewPlayer);

    //ANumGameStateBase* NumGameStateBase = Cast<ANumGameStateBase>(GameState);
    //if (NumGameStateBase)
	//{
	//	NumGameStateBase->MulticastRPCBroadcastLoginMessage(TEXT("XXXXXXX"));
	//}
	//ANumPlayerController* NumPlayerController = Cast<ANumPlayerController>(NewPlayer);
	//if (IsValid(NumPlayerController) == true)
	//{
	//	AllPlayerControllers.Add(NumPlayerController);
	//}
	ANumPlayerController* NumPlayerController = Cast<ANumPlayerController>(NewPlayer);
	if (IsValid(NumPlayerController) == true)
	{
		NumPlayerController->NotificationText = FText::FromString(TEXT("Connected to the game server."));

		AllPlayerControllers.Add(NumPlayerController);

		ANumPlayerState* CXPS = NumPlayerController->GetPlayerState<ANumPlayerState>();
		if (IsValid(CXPS) == true)
		{
			CXPS->PlayerNameString = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());
		}

		ANumGameStateBase* NumGameStateBase = GetGameState<ANumGameStateBase>();
		if (IsValid(NumGameStateBase) == true)
		{
			NumGameStateBase->MulticastRPCBroadcastLoginMessage(CXPS->PlayerNameString);
		}
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
		// The game uses digits from 1 through 9. Uniqueness is checked separately
		// so the caller can show a more specific message for repeated digits.
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

	// Check the attempt limit first so it takes priority over all input errors.
	if (PlayerState->CurrentGuessCount >= PlayerState->MaxGuessCount)
	{
		InChattingPlayerController->ClientRPCSetChatInputEnabled(false);
		const FString NoAttemptsMessage = PlayerState->GetPlayerInfoString()
			+ TEXT(": 기회를 모두 사용했습니다.");
		for (TActorIterator<ANumPlayerController> It(GetWorld()); It; ++It)
		{
			ANumPlayerController* NumPlayerController = *It;
			if (IsValid(NumPlayerController))
			{
				NumPlayerController->ClientRPCPrintChatMessageString(NoAttemptsMessage);
			}
		}
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
	IncreaseGuessCount(InChattingPlayerController);
	const FString JudgeResultString = JudgeResult(SecretNumberString, InChatMessageString);
	const int32 StrikeCount = FCString::Atoi(*JudgeResultString.Left(1));
	const FString CombinedMessageString = PlayerState->GetPlayerInfoString()
		+ TEXT(": ") + InChatMessageString + TEXT(" -> ") + JudgeResultString;
	for (TActorIterator<ANumPlayerController> It(GetWorld()); It; ++It)
	{
		ANumPlayerController* NumPlayerController = *It;
		if (IsValid(NumPlayerController) == true)
		{
			NumPlayerController->ClientRPCPrintChatMessageString(CombinedMessageString);
		}
	}

	if (StrikeCount < 3 && PlayerState->CurrentGuessCount >= PlayerState->MaxGuessCount)
	{
		InChattingPlayerController->ClientRPCSetChatInputEnabled(false);
		const FString NoAttemptsMessage = PlayerState->GetPlayerInfoString()
			+ TEXT(": 기회를 모두 사용했습니다.");
		for (TActorIterator<ANumPlayerController> It(GetWorld()); It; ++It)
		{
			ANumPlayerController* NumPlayerController = *It;
			if (IsValid(NumPlayerController))
			{
				NumPlayerController->ClientRPCPrintChatMessageString(NoAttemptsMessage);
			}
		}
	}

	JudgeGame(InChattingPlayerController, StrikeCount);
}

void ANumGameModeBase::IncreaseGuessCount(ANumPlayerController* InChattingPlayerController)
{
	ANumPlayerState* CXPS = InChattingPlayerController->GetPlayerState<ANumPlayerState>();
	if (IsValid(CXPS) == true)
	{
		CXPS->CurrentGuessCount++;
	}
}

void ANumGameModeBase::ResetGame()
{
	SecretNumberString = GenerateSecretNumber();
	SubmittedGuesses.Reset();
	UE_LOG(LogTemp, Warning, TEXT("라운드 정답: %s"), *SecretNumberString);

	for (const auto& NumPlayerController : AllPlayerControllers)
	{
		if (!IsValid(NumPlayerController))
		{
			continue;
		}

		ANumPlayerState* CXPS = NumPlayerController->GetPlayerState<ANumPlayerState>();
		if (IsValid(CXPS) == true)
		{
			CXPS->CurrentGuessCount = 0;
		}
		NumPlayerController->ClientRPCSetChatInputEnabled(true);
	}
}

void ANumGameModeBase::JudgeGame(ANumPlayerController* InChattingPlayerController, int InStrikeCount)
{
	if (3 == InStrikeCount)
	{
		ANumPlayerState* CXPS = InChattingPlayerController->GetPlayerState<ANumPlayerState>();
		if (!IsValid(CXPS))
		{
			return;
		}

		for (const auto& NumPlayerController : AllPlayerControllers)
		{
			if (IsValid(NumPlayerController) == true)
			{
				FString CombinedMessageString = CXPS->PlayerNameString + TEXT(" has won the game.");
				NumPlayerController->NotificationText = FText::FromString(CombinedMessageString);
			}
		}
		ResetGame();
	}
	else
	{
		bool bIsDraw = true;
		for (const auto& NumPlayerController : AllPlayerControllers)
		{
			ANumPlayerState* CXPS = NumPlayerController->GetPlayerState<ANumPlayerState>();
			if (IsValid(CXPS) == true)
			{
				if (CXPS->CurrentGuessCount < CXPS->MaxGuessCount)
				{
					bIsDraw = false;
					break;
				}
			}
		}

		if (true == bIsDraw)
		{
			for (const auto& NumPlayerController : AllPlayerControllers)
			{
				if (IsValid(NumPlayerController) == true)
				{
					NumPlayerController->NotificationText = FText::FromString(TEXT("Draw..."));
				}
			}
			ResetGame();
		}
	}
}
