#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SearchEscapeHUDWidget.generated.h"

class ASearchEscapeGameMode;
class UBorder;
class UButton;
class UCanvasPanel;
class UProgressBar;
class UTextBlock;
class UVerticalBox;
class UWidget;

UCLASS(Blueprintable)
class AGIS0_51_API USearchEscapeHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable, Category = "Search Escape UI")
    void SetGameMode(ASearchEscapeGameMode* InGameMode);

    UFUNCTION(BlueprintCallable, Category = "Search Escape UI")
    void ShowStartScreen();

    UFUNCTION(BlueprintCallable, Category = "Search Escape UI")
    void ShowPlayingHUD();

    UFUNCTION(BlueprintCallable, Category = "Search Escape UI")
    void ShowEndScreen(bool bSuccess, int32 FinalGold, const FText& Reason);

    UFUNCTION(BlueprintCallable, Category = "Search Escape UI")
    void SetGold(int32 NewGold);

    UFUNCTION(BlueprintCallable, Category = "Search Escape UI")
    void SetTimeRemaining(float NewTimeRemaining);

    UFUNCTION(BlueprintCallable, Category = "Search Escape UI")
    void SetKillCount(int32 NewKillCount);

    UFUNCTION(BlueprintCallable, Category = "Search Escape UI")
    void SetCombatScore(int32 NewScore);

    UFUNCTION(BlueprintCallable, Category = "Search Escape UI")
    void SetPlayerHealth(float CurrentHealth, float MaxHealth);

protected:
    UFUNCTION()
    void HandleStartClicked();

    UFUNCTION()
    void HandleRestartClicked();

private:
    UPROPERTY()
    TObjectPtr<ASearchEscapeGameMode> OwningGameMode;

    // Layout
    UPROPERTY()
    TObjectPtr<UCanvasPanel> RootCanvas;

    // Start screen
    UPROPERTY()
    TObjectPtr<UBorder> StartPanel;

    // HUD
    UPROPERTY()
    TObjectPtr<UBorder> HUDPanel;

    UPROPERTY()
    TObjectPtr<UTextBlock> GoldText;

    UPROPERTY()
    TObjectPtr<UTextBlock> TimeText;

    UPROPERTY()
    TObjectPtr<UTextBlock> KillCountText;

    UPROPERTY()
    TObjectPtr<UTextBlock> CombatScoreText;

    UPROPERTY()
    TObjectPtr<UProgressBar> HealthBar;

    UPROPERTY()
    TObjectPtr<UTextBlock> HealthText;

    // End screen
    UPROPERTY()
    TObjectPtr<UBorder> EndPanel;

    UPROPERTY()
    TObjectPtr<UTextBlock> EndTitleText;

    UPROPERTY()
    TObjectPtr<UTextBlock> EndGoldText;

    UPROPERTY()
    TObjectPtr<UTextBlock> EndKillCountText;

    UPROPERTY()
    TObjectPtr<UTextBlock> EndCombatScoreText;

    UPROPERTY()
    TObjectPtr<UTextBlock> EndReasonText;

    bool bLayoutBuilt = false;
    int32 FinalGoldCache = 0;
    int32 FinalKillCache = 0;
    int32 FinalScoreCache = 0;

    void BuildLayout();
    UTextBlock* MakeText(const FText& Text, int32 FontSize, const FLinearColor& Color);
    UButton* MakeButton(const FText& Text);
    void AddFullScreenPanel(UBorder* Panel);
    static FText FormatTime(float Seconds);
};
