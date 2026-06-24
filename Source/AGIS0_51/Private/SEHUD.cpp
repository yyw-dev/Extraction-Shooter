#include "SEHUD.h"

#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "SEGameMode.h"
#include "SearchEscapePlayerComponent.h"

void ASEHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	APawn* Pawn = GetOwningPawn();
	USearchEscapePlayerComponent* SEComp = Pawn ? Pawn->FindComponentByClass<USearchEscapePlayerComponent>() : nullptr;
	ASEGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ASEGameMode>() : nullptr;

	const float Padding = 28.0f;
	const FVector2D HealthBarPos(Padding, Padding);
	const FVector2D HealthBarSize(260.0f, 22.0f);
	const float HealthPercent = SEComp ? SEComp->GetHealthPercent() : 0.0f;

	DrawSearchEscapeText(TEXT("HEALTH"), FVector2D(Padding, Padding - 22.0f), FLinearColor::White, 0.9f);
	DrawBar(HealthBarPos, HealthBarSize, HealthPercent, FLinearColor(0.1f, 0.85f, 0.25f, 1.0f), FLinearColor(0.08f, 0.02f, 0.02f, 0.8f));

	const int32 Power = SEComp ? SEComp->SE_CombatPower : 0;
	DrawSearchEscapeText(FString::Printf(TEXT("Power: %d"), Power), FVector2D(Padding, 94.0f), FLinearColor(0.55f, 0.8f, 1.0f, 1.0f), 1.0f);

	const FString TimerText = GameMode ? GameMode->GetTimerText() : TEXT("05:00");
	DrawSearchEscapeText(TimerText, FVector2D(Canvas->ClipX * 0.5f - 48.0f, Padding), FLinearColor::White, 1.35f);

	if (GameMode)
	{
		DrawSearchEscapeText(FString::Printf(TEXT("Round %d"), GameMode->SE_RoundNumber),
			FVector2D(Canvas->ClipX * 0.5f - 40.0f, Padding + 30.0f), FLinearColor(1.0f, 1.0f, 0.0f, 1.0f), 0.9f);
	}

	const FString Objective = GameMode && GameMode->SE_EscapeDoorsActive
		? TEXT("Escape doors active")
		: TEXT("Search chests, fight monsters, survive");
	DrawSearchEscapeText(Objective, FVector2D(Canvas->ClipX * 0.5f - 180.0f, Padding + 38.0f), FLinearColor(0.9f, 0.94f, 1.0f, 1.0f), 0.85f);

	DrawSearchEscapeText(TEXT("LMB: Attack"), FVector2D(Canvas->ClipX - 170.0f, Padding), FLinearColor(0.85f, 0.9f, 0.95f, 1.0f), 0.8f);

	if (GameMode && GameMode->SE_GameEnded)
	{
		FCanvasTileItem Overlay(FVector2D::ZeroVector, FVector2D(Canvas->ClipX, Canvas->ClipY), FLinearColor(0.0f, 0.0f, 0.0f, 0.68f));
		Overlay.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(Overlay);

		const FString Title = GameMode->SE_ResultTitle.ToString();
		const FString Detail = GameMode->SE_ResultDetail.ToString();
		DrawSearchEscapeText(Title, FVector2D(Canvas->ClipX * 0.5f - 155.0f, Canvas->ClipY * 0.5f - 74.0f), FLinearColor::White, 1.6f);
		DrawSearchEscapeText(Detail, FVector2D(Canvas->ClipX * 0.5f - 250.0f, Canvas->ClipY * 0.5f - 24.0f), FLinearColor(0.86f, 0.92f, 1.0f, 1.0f), 0.95f);
		DrawSearchEscapeText(FString::Printf(TEXT("Final Gold: %d"), GameMode->SE_TotalGold), FVector2D(Canvas->ClipX * 0.5f - 95.0f, Canvas->ClipY * 0.5f + 16.0f), FLinearColor(1.0f, 0.86f, 0.25f, 1.0f), 1.15f);
	}
}

void ASEHUD::DrawBar(const FVector2D& Position, const FVector2D& Size, float Percent, const FLinearColor& FillColor, const FLinearColor& BackColor) const
{
	const float ClampedPercent = FMath::Clamp(Percent, 0.0f, 1.0f);
	FCanvasTileItem BackItem(Position, Size, BackColor);
	BackItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(BackItem);

	FCanvasTileItem FillItem(Position + FVector2D(2.0f, 2.0f), FVector2D((Size.X - 4.0f) * ClampedPercent, Size.Y - 4.0f), FillColor);
	FillItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(FillItem);
}

void ASEHUD::DrawSearchEscapeText(const FString& Text, const FVector2D& Position, const FLinearColor& Color, float Scale) const
{
	UFont* Font = GEngine ? GEngine->GetMediumFont() : nullptr;
	if (!Font || !Canvas)
	{
		return;
	}

	FCanvasTextItem TextItem(Position, FText::FromString(Text), Font, Color);
	TextItem.Scale = FVector2D(Scale, Scale);
	TextItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(TextItem);
}
