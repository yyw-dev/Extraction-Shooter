#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SearchEscapeGameMode.generated.h"

class ASearchEscapeChest;
class ASearchEscapeDoor;
class ACharacter;
class UAnimationAsset;
class UAnimInstance;
class USearchEscapeHUDWidget;
class USearchEscapeHealthComponent;
class UInputAction;

UENUM(BlueprintType)
enum class ESearchEscapeRunState : uint8
{
    WaitingToStart,
    Playing,
    Escaped,
    Failed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSearchEscapeGoldChangedSignature, int32, NewGold);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSearchEscapeTimeChangedSignature, float, NewTimeRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSearchEscapeGameEndedSignature, bool, bSuccess, int32, FinalGold, FText, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSearchEscapeKillCountChangedSignature, int32, NewKillCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSearchEscapeCombatScoreChangedSignature, int32, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSearchEscapePlayerHealthChangedSignature, float, NewHealth, float, MaxHealth);

UCLASS(Blueprintable)
class AGIS0_51_API ASearchEscapeGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ASearchEscapeGameMode();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    void SetInputForMenu(bool bMenuInput);

    void OnPauseKeyPressed();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Search Escape|UI")
    TSubclassOf<USearchEscapeHUDWidget> HUDWidgetClass;

    // ---- Rules ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Escape|Rules", meta = (ClampMin = "1.0"))
    float RoundDuration = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Escape|Rules", meta = (ClampMin = "0.0"))
    float EscapeDoorActivationTime = 240.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Escape|Rules")
    bool bShowStartScreen = true;

    // ---- Player Combat ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Escape|Player", meta = (ClampMin = "1.0"))
    float PlayerMaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Escape|Player", meta = (ClampMin = "0"))
    float PlayerAttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Escape|Player", meta = (ClampMin = "0"))
    float PlayerAttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Escape|Player", meta = (ClampMin = "0.1"))
    float PlayerAttackCooldown = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Escape|Player")
    bool bRequireAllEnemiesDeadToEscape = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Escape|Player Animation")
    TSubclassOf<UAnimInstance> CharacterLocomotionAnimClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Escape|Player Animation")
    TObjectPtr<UAnimationAsset> PlayerAttackAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Escape|Player Animation", meta = (ClampMin = "0.1"))
    float PlayerAttackAnimationDuration = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Escape|Player Animation", meta = (ClampMin = "0.0"))
    float PlayerAttackImpactDelay = 0.42f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Escape|Player Combat", meta = (ClampMin = "0.0"))
    float PlayerHitKnockbackStrength = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Search Escape|Player Combat", meta = (ClampMin = "0.0"))
    float PlayerHitKnockbackUpStrength = 100.0f;

    // ---- State ----

    UPROPERTY(BlueprintReadOnly, Category = "Search Escape|State")
    int32 CurrentGold = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Search Escape|State")
    float TimeRemaining = 300.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Search Escape|State")
    ESearchEscapeRunState RunState = ESearchEscapeRunState::WaitingToStart;

    UPROPERTY(BlueprintReadOnly, Category = "Search Escape|State")
    int32 KillCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Search Escape|State")
    int32 CombatScore = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Search Escape|State")
    float PlayerCurrentHealth = 100.0f;

    // ---- Events ----

    UPROPERTY(BlueprintAssignable, Category = "Search Escape|Events")
    FSearchEscapeGoldChangedSignature OnGoldChanged;

    UPROPERTY(BlueprintAssignable, Category = "Search Escape|Events")
    FSearchEscapeTimeChangedSignature OnTimeChanged;

    UPROPERTY(BlueprintAssignable, Category = "Search Escape|Events")
    FSearchEscapeGameEndedSignature OnGameEnded;

    UPROPERTY(BlueprintAssignable, Category = "Search Escape|Events")
    FSearchEscapeKillCountChangedSignature OnKillCountChanged;

    UPROPERTY(BlueprintAssignable, Category = "Search Escape|Events")
    FSearchEscapeCombatScoreChangedSignature OnCombatScoreChanged;

    UPROPERTY(BlueprintAssignable, Category = "Search Escape|Events")
    FSearchEscapePlayerHealthChangedSignature OnPlayerHealthChanged;

    // ---- Functions ----

    UFUNCTION(BlueprintCallable, Category = "Search Escape")
    void StartSearchEscapeGame();

    UFUNCTION(BlueprintCallable, Category = "Search Escape")
    void AddGold(int32 GoldAmount);

    UFUNCTION(BlueprintCallable, Category = "Search Escape")
    void EndSearchEscapeGame(bool bSuccess, const FText& Reason);

    UFUNCTION(BlueprintCallable, Category = "Search Escape")
    void RestartSearchEscapeGame();

    /** Start next round — keeps player pawn and gold, resets everything else */
    UFUNCTION(BlueprintCallable, Category = "Search Escape")
    void NextRound();

    /** Death restart — clears gold and items, spawns fresh pawn */
    UFUNCTION(BlueprintCallable, Category = "Search Escape")
    void DeathRestart();

    UPROPERTY(BlueprintReadOnly, Category = "Search Escape|State")
    int32 RoundNumber = 1;

private:
    /** Saved enemy spawn data for respawning (supports both enemy types) */
    struct FSavedEnemySpawn
    {
        TSubclassOf<AActor> EnemyClass;
        FTransform SpawnTransform;
    };
    TArray<FSavedEnemySpawn> SavedEnemySpawns;
    bool bEnemySpawnsSaved = false;

    UFUNCTION(BlueprintPure, Category = "Search Escape")
    bool IsGamePlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Search Escape")
    void PlayerAttack();

    UFUNCTION(BlueprintCallable, Category = "Search Escape")
    void ApplyDamageToPlayer(float Amount, AActor* DamageInstigator);

    UFUNCTION(BlueprintCallable, Category = "Search Escape|AI")
    int32 GetAliveEnemyCount() const;

protected:
    UPROPERTY()
    TObjectPtr<USearchEscapeHUDWidget> HUDWidget;

    UPROPERTY()
    TObjectPtr<UInputAction> AttackInputAction;

    UPROPERTY()
    TObjectPtr<USearchEscapeHealthComponent> PlayerHealthComponent;

private:
    bool bEscapeDoorsActivated = false;
    bool bMenuInputApplied = false;
    bool bPlayerCanAttack = true;
    bool bPlayerMovementLockedForAttack = false;
    FTimerHandle PlayerAttackCooldownTimer;
    FTimerHandle PlayerAttackAnimationTimer;
    FTimerHandle PlayerAttackImpactTimer;
    FTimerHandle PlayerAttackMovementLockTimer;

    void CreateHUD();
    void SpawnActorsFromMarkers();
    void PrepareGameplayActorsForNewRun();
    void BindGameplayActors();
    void BindEnemyDeathEvents();
    void ActivateEscapeDoors();
    void UpdateHUDState();
    void SetupPlayerAttackInput(APlayerController* PC);
    void RemovePlayerAttackInput(APlayerController* PC);
    void SetupPlayerHealth();
    void ConfigurePlayerAnimation();
    void PlayPlayerAttackAnimation();
    void ResolvePlayerAttackImpact();
    void LockPlayerForAttack();
    void UnlockPlayerFromAttack();
    void RestorePlayerLocomotionAnimation();
    ACharacter* GetPlayerCharacter() const;
    void OnPlayerAttackCooldownEnd();

    UFUNCTION()
    void HandleChestOpened(ASearchEscapeChest* Chest, AActor* Player, int32 GoldAmount);

    UFUNCTION()
    void HandleEscapeSucceeded(ASearchEscapeDoor* Door, AActor* Player);

    UFUNCTION()
    void HandlePlayerHealthDepleted(AActor* InOwner, AActor* Killer);

    UFUNCTION()
    void HandlePlayerHealthChanged(AActor* InOwner, float NewHealth, float Delta);

    UFUNCTION()
    void HandleEnemyKilled(AActor* Enemy, AActor* Killer);
};
