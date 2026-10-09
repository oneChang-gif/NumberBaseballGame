#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "NumPawn.generated.h"

UCLASS()
class NUMBERBASEBALLGAME_API ANumPawn : public APawn
{
	GENERATED_BODY()

public:
	ANumPawn();

protected:
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual bool IsLocallyControlled() const override;
};
