#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SEMonster.generated.h"

class UStaticMeshComponent;
class ASEPlayerCharacter;

UCLASS(Blueprintable)
class AGIS0_51_API ASEMonster : public ACharacter
{
	GENERATED_BODY()

public:
	ASEMonster();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "SearchEscape|Monster")
	void SE_TakeDamage(float Amount, ASEPlayerCharacter* InstigatorPlayer);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SearchEscape|Monster")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SearchEscape|Stats")
	float SE_MaxHealth = 60.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SearchEscape|Stats")
	float SE_Health = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SearchEscape|Combat")
	float SE_Damage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SearchEscape|Combat")
	float SE_AttackRange = 165.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SearchEscape|AI")
	float SE_DetectRange = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SearchEscape|AI")
	float SE_PatrolRadius = 650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SearchEscape|AI")
	float SE_ChaseLimit = 2100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SearchEscape|Reward")
	int32 SE_RewardGold = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SearchEscape|Reward")
	int32 SE_RewardCombatPower = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SearchEscape|Stats")
	bool SE_IsDead = false;

protected:
	virtual void BeginPlay() override;

private:
	void UpdateAI(float DeltaSeconds);
	void Patrol();
	void ChasePlayer(ASEPlayerCharacter* Player);
	void AttackPlayer(ASEPlayerCharacter* Player);
	void Die(ASEPlayerCharacter* Killer);
	void PickPatrolPoint();

	FVector SpawnLocation = FVector::ZeroVector;
	FVector PatrolTarget = FVector::ZeroVector;
	float LastAttackTime = -1000.0f;
	float AttackCooldown = 1.25f;
};
