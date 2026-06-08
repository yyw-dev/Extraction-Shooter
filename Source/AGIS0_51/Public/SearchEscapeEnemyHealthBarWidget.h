#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SearchEscapeEnemyHealthBarWidget.generated.h"

class UBorder;
class UProgressBar;

UCLASS(Blueprintable)
class AGIS0_51_API USearchEscapeEnemyHealthBarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual TSharedRef<SWidget> RebuildWidget() override;

    UFUNCTION(BlueprintCallable, Category = "Enemy Health")
    void SetHealthPercent(float NewPercent);

private:
    UPROPERTY()
    TObjectPtr<UBorder> OuterFrame;

    UPROPERTY()
    TObjectPtr<UProgressBar> HealthBar;

    bool bLayoutBuilt = false;
    float PendingPercent = 1.0f;

    void BuildLayout();
};
