#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SEHUD.generated.h"

UCLASS()
class AGIS0_51_API ASEHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void DrawBar(const FVector2D& Position, const FVector2D& Size, float Percent, const FLinearColor& FillColor, const FLinearColor& BackColor) const;
	void DrawSearchEscapeText(const FString& Text, const FVector2D& Position, const FLinearColor& Color, float Scale = 1.0f) const;
};
