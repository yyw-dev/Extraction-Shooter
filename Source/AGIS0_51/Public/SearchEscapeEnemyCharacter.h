#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SearchEscapeEnemyCharacter.generated.h"

class USearchEscapeHealthComponent;
class USearchEscapeEnemyHealthBarWidget;
class UAnimationAsset;
class UAnimInstance;
class USkeletalMesh;
class UWidgetComponent;
class ASEPlayerCharacter;

UENUM(BlueprintType)
enum class ESearchEscapeEnemyState : uint8
{
    Patrol,
    Investigate,
    Chase,
    Attack,
    Dead
};

UCLASS(Blueprintable)
class AGIS0_51_API ASearchEscapeEnemyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ASearchEscapeEnemyCharacter();

    virtual void Tick(float DeltaSeconds) override;
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

    // ---- Components ----

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Components")
    TObjectPtr<USearchEscapeHealthComponent> HealthComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Components")
    TObjectPtr<UWidgetComponent> OverheadWidget;

    // ---- Settings ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat", meta = (ClampMin = "0"))
    float AttackRange = 220.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat", meta = (ClampMin = "0"))
    float AttackDamage = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat", meta = (ClampMin = "0.1"))
    float AttackCooldown = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat", meta = (ClampMin = "0.0"))
    float AttackKnockbackStrength = 420.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat", meta = (ClampMin = "0.0"))
    float AttackKnockbackUpStrength = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI", meta = (ClampMin = "0"))
    float PatrolRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI", meta = (ClampMin = "0"))
    float ChaseRange = 1600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|AI", meta = (ClampMin = "0"))
    float MaxChaseDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Animation")
    TObjectPtr<UAnimationAsset> AttackAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Animation")
    TObjectPtr<UAnimationAsset> IdleAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Animation")
    TObjectPtr<UAnimationAsset> WalkAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Animation")
    TObjectPtr<UAnimationAsset> RunAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Animation")
    TSubclassOf<UAnimInstance> LocomotionAnimClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Animation", meta = (ClampMin = "0.1"))
    float AttackAnimationDuration = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Animation", meta = (ClampMin = "0.0"))
    float AttackImpactDelay = 0.42f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Animation", meta = (ClampMin = "1.0"))
    float RunAnimationSpeedThreshold = 320.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Movement", meta = (ClampMin = "1.0"))
    float PatrolWalkSpeed = 220.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Movement", meta = (ClampMin = "1.0"))
    float ChaseRunSpeed = 480.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Hit Reaction", meta = (ClampMin = "0.0"))
    float KnockbackStrength = 220.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Hit Reaction", meta = (ClampMin = "0.0"))
    float KnockbackUpStrength = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Hit Reaction", meta = (ClampMin = "0.0"))
    float DeathImpulseStrength = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Reward", meta = (ClampMin = "0"))
    int32 KillGoldReward = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Reward", meta = (ClampMin = "0"))
    int32 KillScoreReward = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Debug")
    bool bPrintDebugMessages = true;

    // ---- State ----

    UPROPERTY(BlueprintReadOnly, Category = "Enemy|State")
    ESearchEscapeEnemyState CurrentState = ESearchEscapeEnemyState::Patrol;

    UPROPERTY(BlueprintReadOnly, Category = "Enemy|State")
    bool bCanAttack = true;

    UPROPERTY(BlueprintReadOnly, Category = "Enemy|State")
    FVector PatrolOrigin;

    // ---- Functions ----

    UFUNCTION(BlueprintCallable, Category = "Enemy")
    void SetEnemyState(ESearchEscapeEnemyState NewState);

    UFUNCTION(BlueprintCallable, Category = "Enemy")
    bool CanAttackTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Enemy")
    void PerformAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Enemy")
    void ApplyKnockbackFrom(AActor* Source, float HorizontalStrength, float VerticalStrength);

    UFUNCTION(BlueprintCallable, Category = "Enemy")
    void ResetEnemyForRun();

    // Blueprint-compatible damage entry point — matches the signature that AGIS weapon system calls
    UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
    void SE_TakeDamage(float Amount, ASEPlayerCharacter* InstigatorPlayer);

    UFUNCTION(BlueprintPure, Category = "Enemy")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Enemy")
    bool IsDead() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Events")
    void ReceiveEnemyStateChanged(ESearchEscapeEnemyState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Events")
    void ReceiveAttackPerformed(AActor* Target);

    UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Events")
    void ReceiveEnemyDied(AActor* Killer);

protected:
    virtual void BeginPlay() override;

private:
    FTimerHandle AttackCooldownTimer;
    FTimerHandle AttackAnimationTimer;
    FTimerHandle AttackImpactTimer;
    TWeakObjectPtr<AActor> PendingAttackTarget;
    FVector LastKnownPlayerLocation;
    FTransform InitialMeshRelativeTransform;
    TObjectPtr<UAnimationAsset> CurrentLoopingAnimation;
    bool bIsPlayingAttackAnimation = false;

    UFUNCTION()
    void OnHealthDepleted(AActor* InOwner, AActor* Killer);

    UFUNCTION()
    void OnHealthValueChanged(AActor* InOwner, float NewHealth, float Delta);

    void OnAttackCooldownEnd();

    void ApplyMovementSettingsForState(ESearchEscapeEnemyState NewState);
    void ConfigureDefaultMeshIfNeeded();
    void ConfigureOverheadHealthBar();
    void UpdateOverheadHealthBar();
    void UpdateOverheadHealthBarVisibility();
    void FaceOverheadHealthBarToCamera();
    void UpdateLocomotionAnimation();
    void PlayLoopingAnimation(UAnimationAsset* Animation);
    void PlayAttackAnimation();
    void ResolveAttackImpact();
    bool IsTargetStillInAttackRange(AActor* Target) const;
    void ApplyKnockbackToTarget(AActor* Target) const;
    void RestoreLocomotionAnimation();
    void EnableRagdoll(AActor* Killer);

    void Die(AActor* Killer);
};
