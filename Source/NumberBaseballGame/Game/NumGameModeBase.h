#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"
#include "NumGameModeBase.generated.h"

class ANumPlayerController;

UCLASS()
class NUMBERBASEBALLGAME_API ANumGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	virtual void OnPostLogin(AController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	void PrintChatMessageString(ANumPlayerController* InChattingPlayerController, const FString& InChatMessageString);

	void ResetGame();

	void JudgeGame(ANumPlayerController* InChattingPlayerController, int InStrikeCount);

	bool IsGuessNumberString(const FString& InNumberString);

	FString GenerateSecretNumber();

	FString JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString);

	void IncreaseGuessCount(ANumPlayerController* InChattingPlayerController);

protected:
	void StartTurnSystem();
	void StartNextTurn();
	void UpdateTurnClock();
	void FinishCurrentTurn();
	void StopTurnSystem();
	bool CanSubmitDuringTurn(ANumPlayerController* Player) const;

	UPROPERTY(EditDefaultsOnly, Category = "Turn Practice", meta = (ClampMin = "1"))
	int32 TurnDurationSeconds = 15;

	FTimerHandle TurnClockHandle;
	int32 CurrentTurnPlayerIndex = INDEX_NONE;
	bool bSubmittedThisTurn = false;

	FString SecretNumberString;
	TSet<FString> SubmittedGuesses;

	UPROPERTY()
	TArray<TObjectPtr<ANumPlayerController>> AllPlayerControllers;
};
