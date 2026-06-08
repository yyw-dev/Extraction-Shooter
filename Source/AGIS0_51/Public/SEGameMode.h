#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SEGameMode.generated.h"

class ASEEscapeDoor;

UCLASS(Blueprintable)
class AGIS0_51_API ASEGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASEGameMode();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "SearchEscape|Round")
	void SE_StartRound();

	UFUNCTION(BlueprintCallable, Category = "SearchEscape|Round")
	void SE_EndGameEscape();

	UFUNCTION(BlueprintCallable, Category = "SearchEscape|Round")
	void SE_EndGameDead();

	UFUNCTION(BlueprintCallable, Category = "SearchEscape|Round")
	void SE_EndGameTimeOut();

	UFUNCTION(BlueprintPure, Category = "SearchEscape|Round")
	FString GetTimerText() const;

	UFUNCTION(BlueprintPure, Category = "SearchEscape|Round")
	float GetRoundProgress() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SearchEscape|Round")
	int32 SE_RoundDuration = 300;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SearchEscape|Round")
	int32 SE_EscapeDoorSpawnTime = 240;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SearchEscape|Round")
	int32 SE_TimeRemaining = 300;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SearchEscape|Round")
	int32 SE_TotalGold = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SearchEscape|Round")
	bool SE_GameEnded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SearchEscape|Round")
	bool SE_EscapeDoorsActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SearchEscape|Round")
	FText SE_ResultTitle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SearchEscape|Round")
	FText SE_ResultDetail;

private:
	void TickRoundTimer();
	void SpawnActorsFromMarkers();
	void ActivateEscapeDoors();
	void EndGame(const FText& Title, const FText& Detail);

	FTimerHandle RoundTimerHandle;
	TArray<TObjectPtr<ASEEscapeDoor>> EscapeDoors;
};
