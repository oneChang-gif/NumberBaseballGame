#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NumTimeWidget.generated.h"

class UTextBlock;

UCLASS()
class NUMBERBASEBALLGAME_API UNumTimeWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	void RefreshTimeText();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TurnCheckText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RemainingTurnTimeText;
};
