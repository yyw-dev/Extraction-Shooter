#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SearchEscapePlayerComponent.generated.h"

UCLASS(ClassGroup = (SearchEscape), meta = (BlueprintSpawnableComponent))
class AGIS0_51_API USearchEscapePlayerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USearchEscapePlayerComponent();

	// ---- Health ----
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SE|Stats")
	float SE_MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SE|Stats")
	float SE_Health = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SE|Stats")
	bool SE_IsDead = false;

	UFUNCTION(BlueprintCallable, Category = "SE|Combat")
	void SE_TakeDamage(float Amount);

	UFUNCTION(BlueprintPure, Category = "SE|Stats")
	float GetHealthPercent() const;

	// ---- Gold ----
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SE|Stats")
	int32 SE_Gold = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SE|Stats")
	int32 SE_StartingGold = 0;

	UFUNCTION(BlueprintCallable, Category = "SE|Gold")
	void SE_AddGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "SE|Gold")
	bool SE_SpendGold(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "SE|Gold")
	int32 SE_GetGold() const { return SE_Gold; }

	// ---- Combat Power ----
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SE|Stats")
	int32 SE_CombatPower = 0;

	UFUNCTION(BlueprintCallable, Category = "SE|Stats")
	void SE_AddCombatPower(int32 Amount);

	// ---- Ammo ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SE|Ammo")
	int32 ReserveAmmo = 90;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SE|Ammo", meta = (ClampMin = "1"))
	int32 MaxMagazineAmmo = 30;

	UFUNCTION(BlueprintCallable, Category = "SE|Ammo")
	int32 ReloadWeapon(int32 CurrentBulletCount);

	UFUNCTION(BlueprintPure, Category = "SE|Ammo")
	bool CanReload(int32 CurrentBulletCount) const;

	UFUNCTION(BlueprintCallable, Category = "SE|Ammo")
	void AddReserveAmmo(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "SE|Ammo")
	int32 GetAmmoNeededToFill(int32 CurrentBulletCount) const;

	// ---- Interaction ----
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SE|Interaction")
	float InteractDistance = 250.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SE|Interaction")
	float InteractRadius = 50.0f;

	UFUNCTION(BlueprintCallable, Category = "SE|Interaction")
	void Interact();

	UFUNCTION(BlueprintImplementableEvent, Category = "SE|Interaction")
	void OnInteractFound(AActor* Interactable);

	UFUNCTION(BlueprintImplementableEvent, Category = "SE|Interaction")
	void OnInteractMiss();

	// ---- Events ----
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDeath);
	UPROPERTY(BlueprintAssignable, Category = "SE|Events")
	FOnPlayerDeath OnDeath;

	UFUNCTION(BlueprintImplementableEvent, Category = "SE|Events")
	void OnGoldChanged(int32 NewAmount);

protected:
	virtual void BeginPlay() override;

private:
	float LastAttackTime_DEPRECATED = -1000.0f; // kept for compatibility, not used
};
