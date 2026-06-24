#include "SearchEscapeHUDWidget.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Blueprint/WidgetTree.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SearchEscapeGameMode.h"

TSharedRef<SWidget> USearchEscapeHUDWidget::RebuildWidget()
{
    if (!bLayoutBuilt) BuildLayout();
    return Super::RebuildWidget();
}

void USearchEscapeHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();
    bIsFocusable = true;
    SetKeyboardFocus();
}

FReply USearchEscapeHUDWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::Y)
    {
        TogglePauseMenu();
        return FReply::Handled();
    }
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void USearchEscapeHUDWidget::SetGameMode(ASearchEscapeGameMode* InGameMode)
{
    OwningGameMode = InGameMode;
}

void USearchEscapeHUDWidget::BuildLayout()
{
    bLayoutBuilt = true;

    RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
    WidgetTree->RootWidget = RootCanvas;

    // ==================== START SCREEN ====================
    StartPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("StartPanel"));
    StartPanel->SetBrushColor(FLinearColor(0.02f, 0.025f, 0.03f, 0.88f));
    AddFullScreenPanel(StartPanel);

    UVerticalBox* StartBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StartBox"));
    StartPanel->SetContent(StartBox);

    UTextBlock* Title = MakeText(FText::FromString(TEXT("搜打撤训练场")), 48, FLinearColor::White);
    StartBox->AddChildToVerticalBox(Title)->SetHorizontalAlignment(HAlign_Center);

    UTextBlock* Subtitle = MakeText(FText::FromString(TEXT("搜刮物资，击败敌人，坚持到第 4 分钟后寻找逃生门撤离")), 22, FLinearColor(0.82f, 0.9f, 1.0f, 1.0f));
    UVerticalBoxSlot* SubtitleSlot = StartBox->AddChildToVerticalBox(Subtitle);
    SubtitleSlot->SetHorizontalAlignment(HAlign_Center);
    SubtitleSlot->SetPadding(FMargin(0.0f, 18.0f, 0.0f, 36.0f));

    UButton* StartButton = MakeButton(FText::FromString(TEXT("开始游戏")));
    StartButton->OnClicked.AddDynamic(this, &USearchEscapeHUDWidget::HandleStartClicked);
    UVerticalBoxSlot* StartButtonSlot = StartBox->AddChildToVerticalBox(StartButton);
    StartButtonSlot->SetHorizontalAlignment(HAlign_Center);

    UButton* QuitButton1 = MakeButton(FText::FromString(TEXT("退出游戏")));
    QuitButton1->OnClicked.AddDynamic(this, &USearchEscapeHUDWidget::HandleQuitClicked);
    UVerticalBoxSlot* QuitSlot1 = StartBox->AddChildToVerticalBox(QuitButton1);
    QuitSlot1->SetHorizontalAlignment(HAlign_Center);
    QuitSlot1->SetPadding(FMargin(0.0f, 12.0f, 0.0f, 0.0f));

    // ==================== HUD PANEL ====================
    HUDPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("HUDPanel"));
    HUDPanel->SetBrushColor(FLinearColor(0.01f, 0.012f, 0.016f, 0.78f));
    RootCanvas->AddChild(HUDPanel);
    if (UCanvasPanelSlot* HUDSlot = Cast<UCanvasPanelSlot>(HUDPanel->Slot))
    {
        HUDSlot->SetAnchors(FAnchors(1.0f, 0.0f, 1.0f, 0.0f));
        HUDSlot->SetAlignment(FVector2D(1.0f, 0.0f));
        HUDSlot->SetPosition(FVector2D(-28.0f, 28.0f));
        HUDSlot->SetSize(FVector2D(380.0f, 280.0f));
        HUDSlot->SetZOrder(10);
    }

    UVerticalBox* HUDBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("HUDBox"));
    HUDPanel->SetContent(HUDBox);

    GoldText = MakeText(FText::FromString(TEXT("回合: 1")), 24, FLinearColor(1.0f, 0.82f, 0.22f, 1.0f));
    HUDBox->AddChildToVerticalBox(GoldText)->SetPadding(FMargin(18.0f, 14.0f, 18.0f, 2.0f));

    UTextBlock* Separator = MakeText(FText::FromString(TEXT("----------------")), 12, FLinearColor(0.3f, 0.3f, 0.35f, 1.0f));
    HUDBox->AddChildToVerticalBox(Separator)->SetPadding(FMargin(18.0f, 6.0f, 18.0f, 2.0f));

    HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthBar"));
    HealthBar->SetFillColorAndOpacity(FLinearColor(0.9f, 0.2f, 0.2f, 1.0f));
    USizeBox* HealthBarSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("HealthBarSizeBox"));
    HealthBarSizeBox->SetWidthOverride(260.0f);
    HealthBarSizeBox->SetHeightOverride(12.0f);
    HealthBarSizeBox->SetContent(HealthBar);
    UVerticalBoxSlot* HealthBarSlot = HUDBox->AddChildToVerticalBox(HealthBarSizeBox);
    HealthBarSlot->SetPadding(FMargin(18.0f, 4.0f, 18.0f, 2.0f));

    HealthText = MakeText(FText::FromString(TEXT("生命: 100/100")), 18, FLinearColor(1.0f, 0.85f, 0.85f, 1.0f));
    HUDBox->AddChildToVerticalBox(HealthText)->SetPadding(FMargin(18.0f, 1.0f, 18.0f, 2.0f));

    TimeText = MakeText(FText::FromString(TEXT("时间: 05:00")), 24, FLinearColor(0.72f, 0.92f, 1.0f, 1.0f));
    HUDBox->AddChildToVerticalBox(TimeText)->SetPadding(FMargin(18.0f, 6.0f, 18.0f, 14.0f));

    // ==================== END SCREEN ====================
    EndPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("EndPanel"));
    EndPanel->SetBrushColor(FLinearColor(0.02f, 0.025f, 0.03f, 0.9f));
    AddFullScreenPanel(EndPanel);

    UVerticalBox* EndBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("EndBox"));
    EndPanel->SetContent(EndBox);

    EndTitleText = MakeText(FText::FromString(TEXT("撤离成功")), 48, FLinearColor::Green);
    EndBox->AddChildToVerticalBox(EndTitleText)->SetHorizontalAlignment(HAlign_Center);

    EndReasonText = MakeText(FText::FromString(TEXT("")), 22, FLinearColor(0.84f, 0.88f, 0.95f, 1.0f));
    EndBox->AddChildToVerticalBox(EndReasonText)->SetHorizontalAlignment(HAlign_Center);
    EndReasonText->SetMargin(FMargin(0.0f, 10.0f));

    UButton* RestartButton = MakeButton(FText::FromString(TEXT("重新开始")));
    RestartButton->OnClicked.AddDynamic(this, &USearchEscapeHUDWidget::HandleRestartClicked);
    UVerticalBoxSlot* RestartSlot = EndBox->AddChildToVerticalBox(RestartButton);
    RestartSlot->SetHorizontalAlignment(HAlign_Center);
    RestartSlot->SetPadding(FMargin(0.0f, 30.0f, 0.0f, 0.0f));

    UButton* QuitButton2 = MakeButton(FText::FromString(TEXT("退出游戏")));
    QuitButton2->OnClicked.AddDynamic(this, &USearchEscapeHUDWidget::HandleQuitClicked);
    UVerticalBoxSlot* QuitSlot2 = EndBox->AddChildToVerticalBox(QuitButton2);
    QuitSlot2->SetHorizontalAlignment(HAlign_Center);
    QuitSlot2->SetPadding(FMargin(0.0f, 12.0f, 0.0f, 0.0f));

    // ==================== ROUND COMPLETE SCREEN ====================
    RoundCompletePanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RoundCompletePanel"));
    RoundCompletePanel->SetBrushColor(FLinearColor(0.02f, 0.05f, 0.02f, 0.9f));
    RoundCompletePanel->SetVisibility(ESlateVisibility::Hidden);
    AddFullScreenPanel(RoundCompletePanel);

    UVerticalBox* RoundBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RoundBox"));
    RoundCompletePanel->SetContent(RoundBox);

    UTextBlock* RoundTitle = MakeText(FText::FromString(TEXT("撤离成功！")), 48, FLinearColor::Green);
    RoundBox->AddChildToVerticalBox(RoundTitle)->SetHorizontalAlignment(HAlign_Center);

    UTextBlock* RoundSub = MakeText(FText::FromString(TEXT("物品已保留到下一轮")), 24, FLinearColor(0.82f, 0.9f, 1.0f, 1.0f));
    UVerticalBoxSlot* RoundSubSlot = RoundBox->AddChildToVerticalBox(RoundSub);
    RoundSubSlot->SetHorizontalAlignment(HAlign_Center);
    RoundSubSlot->SetPadding(FMargin(0.0f, 16.0f, 0.0f, 30.0f));

    UButton* NextRoundBtn = MakeButton(FText::FromString(TEXT("开始下一轮")));
    NextRoundBtn->OnClicked.AddDynamic(this, &USearchEscapeHUDWidget::HandleNextRoundClicked);
    UVerticalBoxSlot* NextSlot = RoundBox->AddChildToVerticalBox(NextRoundBtn);
    NextSlot->SetHorizontalAlignment(HAlign_Center);

    UButton* QuitRoundBtn = MakeButton(FText::FromString(TEXT("退出游戏")));
    QuitRoundBtn->OnClicked.AddDynamic(this, &USearchEscapeHUDWidget::HandleQuitClicked);
    UVerticalBoxSlot* QuitRoundSlot = RoundBox->AddChildToVerticalBox(QuitRoundBtn);
    QuitRoundSlot->SetHorizontalAlignment(HAlign_Center);
    QuitRoundSlot->SetPadding(FMargin(0.0f, 12.0f, 0.0f, 0.0f));

    // ==================== PAUSE MENU (Y key) ====================
    PauseMenuPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("PauseMenuPanel"));
    PauseMenuPanel->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f));
    PauseMenuPanel->SetVisibility(ESlateVisibility::Hidden);
    AddFullScreenPanel(PauseMenuPanel);

    UVerticalBox* PauseBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("PauseBox"));
    PauseMenuPanel->SetContent(PauseBox);

    UButton* ContinueBtn = MakeButton(FText::FromString(TEXT("继续游戏")));
    ContinueBtn->OnClicked.AddDynamic(this, &USearchEscapeHUDWidget::HandleContinueClicked);
    UVerticalBoxSlot* ContinueSlot = PauseBox->AddChildToVerticalBox(ContinueBtn);
    ContinueSlot->SetHorizontalAlignment(HAlign_Center);

    UButton* QuitPauseBtn = MakeButton(FText::FromString(TEXT("退出游戏")));
    QuitPauseBtn->OnClicked.AddDynamic(this, &USearchEscapeHUDWidget::HandleQuitClicked);
    UVerticalBoxSlot* QuitPauseSlot = PauseBox->AddChildToVerticalBox(QuitPauseBtn);
    QuitPauseSlot->SetHorizontalAlignment(HAlign_Center);
    QuitPauseSlot->SetPadding(FMargin(0.0f, 12.0f, 0.0f, 0.0f));

    ShowStartScreen();
}

UTextBlock* USearchEscapeHUDWidget::MakeText(const FText& Text, int32 FontSize, const FLinearColor& Color)
{
    UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
    TextBlock->SetText(Text);
    FSlateFontInfo Font = TextBlock->GetFont();
    Font.Size = FontSize;
    TextBlock->SetFont(Font);
    TextBlock->SetColorAndOpacity(FSlateColor(Color));
    TextBlock->SetJustification(ETextJustify::Center);
    return TextBlock;
}

UButton* USearchEscapeHUDWidget::MakeButton(const FText& Text)
{
    UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
    Button->SetBackgroundColor(FLinearColor(0.1f, 0.28f, 0.45f, 1.0f));
    UTextBlock* ButtonText = MakeText(Text, 26, FLinearColor::White);
    ButtonText->SetMargin(FMargin(32.0f, 12.0f));
    Button->SetContent(ButtonText);
    return Button;
}

void USearchEscapeHUDWidget::AddFullScreenPanel(UBorder* Panel)
{
    RootCanvas->AddChild(Panel);
    if (UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(Panel->Slot))
    {
        PanelSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
        PanelSlot->SetOffsets(FMargin(0.0f));
        PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    }
    Panel->SetPadding(FMargin(40.0f));
    Panel->SetHorizontalAlignment(HAlign_Center);
    Panel->SetVerticalAlignment(VAlign_Center);
}

void USearchEscapeHUDWidget::ShowStartScreen()
{
    if (!bLayoutBuilt) return;
    StartPanel->SetVisibility(ESlateVisibility::Visible);
    HUDPanel->SetVisibility(ESlateVisibility::Hidden);
    EndPanel->SetVisibility(ESlateVisibility::Hidden);
    RoundCompletePanel->SetVisibility(ESlateVisibility::Hidden);
    PauseMenuPanel->SetVisibility(ESlateVisibility::Hidden);
}

void USearchEscapeHUDWidget::ShowPlayingHUD()
{
    if (!bLayoutBuilt) return;
    StartPanel->SetVisibility(ESlateVisibility::Hidden);
    HUDPanel->SetVisibility(ESlateVisibility::HitTestInvisible);
    EndPanel->SetVisibility(ESlateVisibility::Hidden);
    RoundCompletePanel->SetVisibility(ESlateVisibility::Hidden);
    PauseMenuPanel->SetVisibility(ESlateVisibility::Hidden);
}

void USearchEscapeHUDWidget::ShowRoundCompleteScreen()
{
    if (!bLayoutBuilt) return;
    StartPanel->SetVisibility(ESlateVisibility::Hidden);
    HUDPanel->SetVisibility(ESlateVisibility::Hidden);
    EndPanel->SetVisibility(ESlateVisibility::Hidden);
    RoundCompletePanel->SetVisibility(ESlateVisibility::Visible);
    PauseMenuPanel->SetVisibility(ESlateVisibility::Hidden);
}

void USearchEscapeHUDWidget::ShowEndScreen(bool bSuccess, int32 FinalGold, const FText& Reason)
{
    if (!bLayoutBuilt) return;
    EndTitleText->SetText(bSuccess ? FText::FromString(TEXT("撤离成功")) : FText::FromString(TEXT("撤离失败")));
    EndTitleText->SetColorAndOpacity(FSlateColor(bSuccess ? FLinearColor(0.35f, 1.0f, 0.45f, 1.0f) : FLinearColor(1.0f, 0.25f, 0.25f, 1.0f)));
    EndReasonText->SetText(Reason);
    StartPanel->SetVisibility(ESlateVisibility::Hidden);
    HUDPanel->SetVisibility(ESlateVisibility::Hidden);
    EndPanel->SetVisibility(ESlateVisibility::Visible);
    RoundCompletePanel->SetVisibility(ESlateVisibility::Hidden);
    PauseMenuPanel->SetVisibility(ESlateVisibility::Hidden);
}

void USearchEscapeHUDWidget::SetGold(int32 NewGold)
{
    if (GoldText && OwningGameMode)
        GoldText->SetText(FText::FromString(FString::Printf(TEXT("回合: %d"), OwningGameMode->RoundNumber)));
}

void USearchEscapeHUDWidget::SetTimeRemaining(float NewTimeRemaining)
{
    if (TimeText)
        TimeText->SetText(FText::Format(FText::FromString(TEXT("时间: {0}")), FormatTime(NewTimeRemaining)));
}

void USearchEscapeHUDWidget::SetKillCount(int32 NewKillCount) {}
void USearchEscapeHUDWidget::SetCombatScore(int32 NewScore) {}

void USearchEscapeHUDWidget::SetPlayerHealth(float CurrentHealth, float MaxHealth)
{
    if (HealthBar)
    {
        HealthBar->SetPercent(MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f);
        const float Pct = HealthBar->GetPercent();
        HealthBar->SetFillColorAndOpacity(FLinearColor(FMath::Lerp(0.9f, 0.2f, 1.0f - Pct), FMath::Lerp(0.2f, 0.05f, 1.0f - Pct), 0.1f, 1.0f));
    }
    if (HealthText)
        HealthText->SetText(FText::FromString(FString::Printf(TEXT("生命: %.0f/%.0f"), CurrentHealth, MaxHealth)));
}

FText USearchEscapeHUDWidget::FormatTime(float Seconds)
{
    const int32 WholeSeconds = FMath::Max(0, FMath::CeilToInt(Seconds));
    return FText::FromString(FString::Printf(TEXT("%02d:%02d"), WholeSeconds / 60, WholeSeconds % 60));
}

void USearchEscapeHUDWidget::HandleStartClicked()
{
    if (OwningGameMode) OwningGameMode->StartSearchEscapeGame();
}

void USearchEscapeHUDWidget::HandleRestartClicked()
{
    if (OwningGameMode) OwningGameMode->RestartSearchEscapeGame();
}

void USearchEscapeHUDWidget::HandleQuitClicked()
{
    if (APlayerController* PC = GetOwningPlayer())
        UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
}

void USearchEscapeHUDWidget::HandleNextRoundClicked()
{
    if (!OwningGameMode) return;
    RoundCompletePanel->SetVisibility(ESlateVisibility::Hidden);
    HUDPanel->SetVisibility(ESlateVisibility::HitTestInvisible);
    OwningGameMode->NextRound();
    if (APlayerController* PC = GetOwningPlayer())
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->SetShowMouseCursor(false);
        PC->ResetIgnoreMoveInput();
        PC->ResetIgnoreLookInput();
    }
}

void USearchEscapeHUDWidget::TogglePauseMenu()
{
    if (!bLayoutBuilt || !PauseMenuPanel) return;
    if (APlayerController* PC = GetOwningPlayer())
    {
        const bool bVisible = PauseMenuPanel->GetVisibility() != ESlateVisibility::Hidden;
        if (bVisible)
        {
            PauseMenuPanel->SetVisibility(ESlateVisibility::Hidden);
            PC->SetInputMode(FInputModeGameOnly());
            PC->SetShowMouseCursor(false);
            PC->ResetIgnoreMoveInput();
            PC->ResetIgnoreLookInput();
        }
        else
        {
            PauseMenuPanel->SetVisibility(ESlateVisibility::Visible);
            FInputModeUIOnly InputMode;
            InputMode.SetWidgetToFocus(TakeWidget());
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PC->SetInputMode(InputMode);
            PC->SetShowMouseCursor(true);
            PC->SetIgnoreMoveInput(true);
            PC->SetIgnoreLookInput(true);
        }
    }
}

void USearchEscapeHUDWidget::HandleContinueClicked()
{
    if (PauseMenuPanel) PauseMenuPanel->SetVisibility(ESlateVisibility::Hidden);
    if (APlayerController* PC = GetOwningPlayer())
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->SetShowMouseCursor(false);
        PC->ResetIgnoreMoveInput();
        PC->ResetIgnoreLookInput();
    }
}
