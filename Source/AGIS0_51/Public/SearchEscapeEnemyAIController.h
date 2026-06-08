#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "SearchEscapeEnemyAIController.generated.h"

class ASearchEscapeEnemyCharacter;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UBehaviorTree;
class UBlackboardComponent;

UCLASS(Blueprintable)
class AGIS0_51_API ASearchEscapeEnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    ASearchEscapeEnemyAIController();

    virtual void OnPossess(APawn* InPawn) override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Blackboard")
    FName TargetActorKey = TEXT("TargetActor");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Blackboard")
    FName TargetLocationKey = TEXT("TargetLocation");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Blackboard")
    FName PatrolLocationKey = TEXT("PatrolLocation");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Blackboard")
    FName EnemyStateKey = TEXT("EnemyState");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Blackboard")
    FName HasLineOfSightKey = TEXT("HasLineOfSight");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
    float SightRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
    float LoseSightRadius = 1400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
    float PeripheralVisionAngleDegrees = 95.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Behavior")
    bool bUseCodeDrivenMovement = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Behavior")
    bool bUseBehaviorTree = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Behavior")
    TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol", meta = (ClampMin = "10.0"))
    float PatrolAcceptanceRadius = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol", meta = (ClampMin = "0.0"))
    float PatrolWaitTime = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Chase", meta = (ClampMin = "0.0"))
    float LostSightChaseTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Chase", meta = (ClampMin = "0.0"))
    float InvestigateWaitTime = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Chase", meta = (ClampMin = "0.05"))
    float MoveRefreshInterval = 0.1f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Components")
    TObjectPtr<UAIPerceptionComponent> EnemyPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Components")
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateBlackboardState();

    UFUNCTION(BlueprintPure, Category = "AI")
    AActor* GetTargetActor() const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TObjectPtr<AActor> CurrentTarget;

    FVector LastKnownTargetLocation = FVector::ZeroVector;
    FVector CurrentPatrolLocation = FVector::ZeroVector;
    bool bHasLineOfSight = false;
    bool bHasPatrolLocation = false;
    float LostSightElapsed = 0.0f;
    float InvestigateElapsed = 0.0f;
    float PatrolWaitRemaining = 0.0f;
    float MoveRefreshRemaining = 0.0f;

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    void ConfigurePerception();
    void StartBehaviorTree();
    void RunCodeDrivenAI(float DeltaSeconds);
    void UpdateChase(float DeltaSeconds, ASearchEscapeEnemyCharacter* Enemy);
    void UpdatePatrol(float DeltaSeconds, ASearchEscapeEnemyCharacter* Enemy);
    bool ChooseReachablePatrolLocation(const ASearchEscapeEnemyCharacter* Enemy, FVector& OutLocation) const;
    void ClearTargetAndReturnToPatrol(ASearchEscapeEnemyCharacter* Enemy);
    void SyncBlackboardTarget();
};
