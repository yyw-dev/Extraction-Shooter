#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SEPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ASEMonster;

UCLASS(Blueprintable)
class AGIS0_51_API ASEPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASEPlayerCharacter();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "SearchEscape|Player")
	void SE_TakeDamage(float Amount);

	UFUNCTION(BlueprintCallable, Category = "SearchEscape|Player")
	void SE_AddGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "SearchEscape|Player")
	void SE_AddCombatPower(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "SearchEscape|Player")
	float GetHealthPercent() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SearchEscape|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SearchEscape|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SearchEscape|Stats")
	float SE_MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SearchEscape|Stats")
	float SE_Health = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SearchEscape|Stats")
	int32 SE_Gold = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SearchEscape|Stats")
	int32 SE_CombatPower = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SearchEscape|Combat")
	float SE_AttackDamage = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SearchEscape|Combat")
	float SE_AttackRange = 260.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SearchEscape|Combat")
	float SE_AttackRadius = 85.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SearchEscape|Combat")
	float SE_AttackCooldown = 0.45f;

	// ---- Simplified Ammo System ----
	// Replaces AGIS magazine-juggling.
	// Blueprint calls ReloadWeapon(currentBulletCount) → gets back new bullet count + consumes reserve.
	// Weapon's own Bullet Count stays the fire-time source of truth (managed by AGIS).

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SearchEscape|Ammo")
	int32 ReserveAmmo = 90;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SearchEscape|Ammo", meta = (ClampMin = "1"))
	int32 MaxMagazineAmmo = 30;

	// Pass the weapon's current bullet count, returns the new count (capped at MaxMagazineAmmo).
	// Returns the same value as input if reload is not possible (mag full or no reserve).
	UFUNCTION(BlueprintCallable, Category = "SearchEscape|Ammo")
	int32 ReloadWeapon(int32 CurrentBulletCount);

	// True if there are bullets missing from the mag AND reserve ammo is available
	UFUNCTION(BlueprintPure, Category = "SearchEscape|Ammo")
	bool CanReload(int32 CurrentBulletCount) const;

	// Add ammo to reserve (e.g. from pickups or consuming a backpack ammo item)
	UFUNCTION(BlueprintCallable, Category = "SearchEscape|Ammo")
	void AddReserveAmmo(int32 Amount);

	// How many bullets are missing from a full magazine
	UFUNCTION(BlueprintPure, Category = "SearchEscape|Ammo")
	int32 GetAmmoNeededToFill(int32 CurrentBulletCount) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SearchEscape|Combat")
	bool bDebugAttackTrace = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SearchEscape|Stats")
	bool SE_IsDead = false;

protected:
	virtual void BeginPlay() override;

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void StartJump();
	void StopJump();
	void Attack();

	float LastAttackTime = -1000.0f;
};
