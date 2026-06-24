#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SearchEscapeArrowProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;
class ASEPlayerCharacter;

UCLASS(Blueprintable)
class AGIS0_51_API ASearchEscapeArrowProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASearchEscapeArrowProjectile();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arrow|Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arrow|Components")
	TObjectPtr<UStaticMeshComponent> ArrowMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Arrow|Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow|Combat")
	float ArrowDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow|Combat")
	float ArrowSpeed = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow|Combat")
	float ArrowGravity = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arrow|Combat")
	float LifetimeSeconds = 8.0f;

	/** Who fired this arrow — won't be damaged by it */
	UPROPERTY(BlueprintReadOnly, Category = "Arrow|State")
	TWeakObjectPtr<AActor> OwningShooter;

	UFUNCTION(BlueprintCallable, Category = "Arrow")
	void FireTowards(const FVector& TargetLocation, AActor* Shooter);

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnArrowHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnArrowOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void ExpireArrow();
};
