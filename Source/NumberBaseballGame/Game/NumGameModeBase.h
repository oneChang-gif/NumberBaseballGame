#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NumGameModeBase.generated.h"

class ANumPlayerController;

UCLASS()
class NUMBERBASEBALLGAME_API ANumGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

	virtual void OnPostLogin(AController* NewPlayer) override;

	void PrintChatMessageString(ANumPlayerController* InChattingPlayerController, const FString& InChatMessageString);

	void ResetGame();

	void JudgeGame(ANumPlayerController* InChattingPlayerController, int InStrikeCount);

	bool IsGuessNumberString(const FString& InNumberString);

	FString GenerateSecretNumber();

	FString JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString);

	void IncreaseGuessCount(ANumPlayerController* InChattingPlayerController);

protected:
	FString SecretNumberString;
	TSet<FString> SubmittedGuesses;

	TArray<TObjectPtr<ANumPlayerController>> AllPlayerControllers;

};
