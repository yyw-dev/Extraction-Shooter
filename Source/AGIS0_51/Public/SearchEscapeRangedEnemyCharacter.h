#pragma once

#include "CoreMinimal.h"
#include "SearchEscapeEnemyCharacter.h"
#include "SearchEscapeRangedEnemyCharacter.generated.h"

class ASearchEscapeArrowProjectile;

UCLASS(Blueprintable)
class AGIS0_51_API ASearchEscapeRangedEnemyCharacter : public ASearchEscapeEnemyCharacter
{
	GENERATED_BODY()

public:
	ASearchEscapeRangedEnemyCharacter();

	/** Projectile class to spawn when attacking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranged|Projectile")
	TSubclassOf<ASearchEscapeArrowProjectile> ArrowProjectileClass;

	/** How far the archer can shoot — also used by AI to decide approach distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranged|Combat", meta = (ClampMin = "500"))
	float MaxShootRange = 2500.0f;

	/** Vertical offset for the arrow spawn point relative to the archer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranged|Combat")
	FVector ArrowSpawnOffset = FVector(80.0f, 0.0f, 180.0f);

	/** Slight random aim offset in degrees to make shots less perfect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranged|Combat", meta = (ClampMin = "0"))
	float AimErrorDegrees = 3.0f;

	/** How long the archer waits between shots */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranged|Combat", meta = (ClampMin = "0.1"))
	float ShootInterval = 1.8f;

	/** Min comfortable distance — if player is closer than this, archer may back up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranged|Behavior", meta = (ClampMin = "0"))
	float MinComfortDistance = 300.0f;

	virtual void Tick(float DeltaSeconds) override;

	/** Override to spawn arrow instead of melee attack */
	virtual void PerformAttack(AActor* Target) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Ranged|Events")
	void ReceiveArrowFired(AActor* Target);

protected:
	virtual void BeginPlay() override;
	virtual void ApplyMovementSettingsForState(ESearchEscapeEnemyState NewState) override;

private:
	float LastShootTime = -1000.0f;

	/** Actually spawn and launch the arrow */
	void FireArrow(AActor* Target);
};
