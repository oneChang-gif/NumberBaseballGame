#include "Player/NumPawn.h"
#include "NumberBaseballGame.h"

ANumPawn::ANumPawn()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ANumPawn::BeginPlay()
{
	Super::BeginPlay();
	
	FString NetRoleString = NumberBaseballGameFunctionLibrary::GetRoleString(this);
	FString CombinedString = FString::Printf(TEXT("CXPawn::BeginPlay() %s [%s]"), *NumberBaseballGameFunctionLibrary::GetNetModeString(this), *NetRoleString);
	NumberBaseballGameFunctionLibrary::MyPrintString(this, CombinedString, 10.f);
}

void ANumPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ANumPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ANumPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	FString NetRoleString = NumberBaseballGameFunctionLibrary::GetRoleString(this);
	FString CombinedString = FString::Printf(TEXT("CXPawn::PossessedBy() %s [%s]"), *NumberBaseballGameFunctionLibrary::GetNetModeString(this), *NetRoleString);
	NumberBaseballGameFunctionLibrary::MyPrintString(this, CombinedString, 10.f);
}

bool ANumPawn::IsLocallyControlled() const
{
	return (Controller && Controller->IsLocalController());
}