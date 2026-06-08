#include "SearchEscapeEnemyHealthBarWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/ProgressBar.h"

TSharedRef<SWidget> USearchEscapeEnemyHealthBarWidget::RebuildWidget()
{
    if (!bLayoutBuilt)
    {
        BuildLayout();
    }

    return Super::RebuildWidget();
}

void USearchEscapeEnemyHealthBarWidget::SetHealthPercent(float NewPercent)
{
    PendingPercent = FMath::Clamp(NewPercent, 0.0f, 1.0f);

    if (HealthBar)
    {
        HealthBar->SetPercent(PendingPercent);
        HealthBar->SetFillColorAndOpacity(FLinearColor(1.0f, 0.02f, 0.02f, 1.0f));
    }
}

void USearchEscapeEnemyHealthBarWidget::BuildLayout()
{
    bLayoutBuilt = true;

    OuterFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("EnemyHealthOuterFrame"));
    OuterFrame->SetBrushColor(FLinearColor(0.08f, 0.0f, 0.0f, 0.95f));
    OuterFrame->SetPadding(FMargin(2.0f));
    WidgetTree->RootWidget = OuterFrame;

    HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("EnemyHealthBar"));
    HealthBar->SetFillColorAndOpacity(FLinearColor(1.0f, 0.02f, 0.02f, 1.0f));
    HealthBar->SetPercent(PendingPercent);
    OuterFrame->SetContent(HealthBar);
}
