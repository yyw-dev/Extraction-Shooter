#include "SearchEscapeEnemyAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "SearchEscapeEnemyCharacter.h"

ASearchEscapeEnemyAIController::ASearchEscapeEnemyAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    EnemyPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("EnemyPerceptionComponent"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    ConfigurePerception();
}

void ASearchEscapeEnemyAIController::BeginPlay()
{
    Super::BeginPlay();

    ConfigurePerception();

    if (EnemyPerceptionComponent)
    {
        EnemyPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ASearchEscapeEnemyAIController::OnTargetPerceptionUpdated);
    }
}

void ASearchEscapeEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (ASearchEscapeEnemyCharacter* Enemy = Cast<ASearchEscapeEnemyCharacter>(InPawn))
    {
        Enemy->PatrolOrigin = Enemy->GetActorLocation();
        Enemy->SetEnemyState(ESearchEscapeEnemyState::Patrol);
    }

    if (bUseBehaviorTree)
    {
        StartBehaviorTree();
    }
}

void ASearchEscapeEnemyAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    UpdateBlackboardState();

    if (bUseCodeDrivenMovement)
    {
        RunCodeDrivenAI(DeltaSeconds);
    }
}

void ASearchEscapeEnemyAIController::ConfigurePerception()
{
    if (!EnemyPerceptionComponent || !SightConfig)
    {
        return;
    }

    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = LoseSightRadius;
    SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;
    SightConfig->SetMaxAge(4.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

    EnemyPerceptionComponent->ConfigureSense(*SightConfig);
    EnemyPerceptionComponent->SetDominantSense(UAISenseConfig_Sight::StaticClass());
    EnemyPerceptionComponent->RequestStimuliListenerUpdate();
}

void ASearchEscapeEnemyAIController::StartBehaviorTree()
{
    if (!BehaviorTreeAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("EnemyAIController: No BehaviorTree assigned for %s"), *GetName());
        return;
    }

    RunBehaviorTree(BehaviorTreeAsset);
}

void ASearchEscapeEnemyAIController::UpdateBlackboardState()
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    APawn* MyPawn = GetPawn();
    if (!BB || !MyPawn)
    {
        return;
    }

    if (ASearchEscapeEnemyCharacter* Enemy = Cast<ASearchEscapeEnemyCharacter>(MyPawn))
    {
        BB->SetValueAsEnum(EnemyStateKey, static_cast<uint8>(Enemy->CurrentState));
    }

    BB->SetValueAsBool(HasLineOfSightKey, bHasLineOfSight);
    BB->SetValueAsVector(TargetLocationKey, LastKnownTargetLocation);
    BB->SetValueAsVector(PatrolLocationKey, CurrentPatrolLocation);

    if (CurrentTarget)
    {
        BB->SetValueAsObject(TargetActorKey, CurrentTarget);
    }
}

AActor* ASearchEscapeEnemyAIController::GetTargetActor() const
{
    return CurrentTarget;
}

void ASearchEscapeEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    APawn* MyPawn = GetPawn();
    APawn* SensedPawn = Cast<APawn>(Actor);
    if (!Actor || Actor == MyPawn || !SensedPawn || !SensedPawn->IsPlayerControlled())
    {
        return;
    }

    ASearchEscapeEnemyCharacter* Enemy = Cast<ASearchEscapeEnemyCharacter>(MyPawn);
    if (!Enemy || Enemy->IsDead())
    {
        return;
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        CurrentTarget = Actor;
        LastKnownTargetLocation = Actor->GetActorLocation();
        bHasLineOfSight = true;
        LostSightElapsed = 0.0f;
        InvestigateElapsed = 0.0f;
        bHasPatrolLocation = false;
        MoveRefreshRemaining = 0.0f;
        Enemy->SetEnemyState(ESearchEscapeEnemyState::Chase);
        SyncBlackboardTarget();

        if (Enemy->bPrintDebugMessages)
        {
            UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s detected %s"), *GetName(), *Actor->GetName()), true, false, FLinearColor::Red, 1.0f);
        }
    }
    else if (CurrentTarget == Actor)
    {
        LastKnownTargetLocation = Actor->GetActorLocation();
        bHasLineOfSight = false;
        LostSightElapsed = 0.0f;
        InvestigateElapsed = 0.0f;
        MoveRefreshRemaining = 0.0f;
        Enemy->SetEnemyState(ESearchEscapeEnemyState::Investigate);
        SyncBlackboardTarget();
    }
}

void ASearchEscapeEnemyAIController::RunCodeDrivenAI(float DeltaSeconds)
{
    ASearchEscapeEnemyCharacter* Enemy = Cast<ASearchEscapeEnemyCharacter>(GetPawn());
    if (!Enemy || Enemy->IsDead())
    {
        StopMovement();
        return;
    }

    MoveRefreshRemaining = FMath::Max(0.0f, MoveRefreshRemaining - DeltaSeconds);

    if (CurrentTarget)
    {
        UpdateChase(DeltaSeconds, Enemy);
    }
    else
    {
        UpdatePatrol(DeltaSeconds, Enemy);
    }
}

void ASearchEscapeEnemyAIController::UpdateChase(float DeltaSeconds, ASearchEscapeEnemyCharacter* Enemy)
{
    if (!CurrentTarget)
    {
        ClearTargetAndReturnToPatrol(Enemy);
        return;
    }

    const float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    const float AllowedChaseDistance = FMath::Max(Enemy->MaxChaseDistance, Enemy->ChaseRange);
    const float TargetDistanceFromOrigin = FVector::Dist2D(CurrentTarget->GetActorLocation(), Enemy->PatrolOrigin);

    if (bHasLineOfSight)
    {
        LastKnownTargetLocation = CurrentTarget->GetActorLocation();
        LostSightElapsed = 0.0f;
        InvestigateElapsed = 0.0f;

        if (DistanceToTarget <= Enemy->AttackRange)
        {
            StopMovement();
            Enemy->SetEnemyState(ESearchEscapeEnemyState::Attack);
            if (Enemy->CanAttackTarget(CurrentTarget))
            {
                Enemy->PerformAttack(CurrentTarget);
            }
        }
        else
        {
            Enemy->SetEnemyState(ESearchEscapeEnemyState::Chase);
            if (MoveRefreshRemaining <= 0.0f)
            {
                // Get much closer than attack range before stopping (50% of attack range instead of 80%)
                MoveToActor(CurrentTarget, Enemy->AttackRange * 0.5f, true, true, true);
                MoveRefreshRemaining = MoveRefreshInterval;
            }
        }
    }
    else
    {
        LostSightElapsed += DeltaSeconds;
        Enemy->SetEnemyState(ESearchEscapeEnemyState::Investigate);

        if (TargetDistanceFromOrigin > AllowedChaseDistance || LostSightElapsed > LostSightChaseTime)
        {
            const float DistanceToLastKnown = FVector::Dist2D(GetPawn()->GetActorLocation(), LastKnownTargetLocation);
            if (DistanceToLastKnown > PatrolAcceptanceRadius)
            {
                if (MoveRefreshRemaining <= 0.0f)
                {
                    MoveToLocation(LastKnownTargetLocation, PatrolAcceptanceRadius);
                    MoveRefreshRemaining = MoveRefreshInterval;
                }
            }
            else
            {
                InvestigateElapsed += DeltaSeconds;
                StopMovement();
            }

            if (InvestigateElapsed >= InvestigateWaitTime)
            {
                ClearTargetAndReturnToPatrol(Enemy);
            }
        }
        else if (MoveRefreshRemaining <= 0.0f)
        {
            MoveToLocation(LastKnownTargetLocation, PatrolAcceptanceRadius);
            MoveRefreshRemaining = MoveRefreshInterval;
        }
    }

    SyncBlackboardTarget();
}

void ASearchEscapeEnemyAIController::UpdatePatrol(float DeltaSeconds, ASearchEscapeEnemyCharacter* Enemy)
{
    Enemy->SetEnemyState(ESearchEscapeEnemyState::Patrol);

    if (PatrolWaitRemaining > 0.0f)
    {
        PatrolWaitRemaining -= DeltaSeconds;
        return;
    }

    const bool bMoveIdle = GetMoveStatus() == EPathFollowingStatus::Idle;
    const bool bReachedPoint = bHasPatrolLocation && FVector::Dist2D(GetPawn()->GetActorLocation(), CurrentPatrolLocation) <= PatrolAcceptanceRadius;

    if (!bHasPatrolLocation || bMoveIdle || bReachedPoint)
    {
        if (bHasPatrolLocation && bReachedPoint)
        {
            bHasPatrolLocation = false;
            PatrolWaitRemaining = PatrolWaitTime;
            return;
        }

        FVector NewLocation;
        if (ChooseReachablePatrolLocation(Enemy, NewLocation))
        {
            CurrentPatrolLocation = NewLocation;
            bHasPatrolLocation = true;
            MoveToLocation(CurrentPatrolLocation, PatrolAcceptanceRadius);
            SyncBlackboardTarget();
        }
    }
}

bool ASearchEscapeEnemyAIController::ChooseReachablePatrolLocation(const ASearchEscapeEnemyCharacter* Enemy, FVector& OutLocation) const
{
    if (!Enemy || !GetWorld())
    {
        return false;
    }

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys)
    {
        OutLocation = Enemy->PatrolOrigin;
        return true;
    }

    const float Radius = FMath::Max(Enemy->PatrolRadius, 300.0f);
    FNavLocation NavLocation;
    if (NavSys->GetRandomReachablePointInRadius(Enemy->PatrolOrigin, Radius, NavLocation))
    {
        OutLocation = NavLocation.Location;
        return true;
    }

    if (NavSys->ProjectPointToNavigation(Enemy->PatrolOrigin, NavLocation, FVector(300.0f, 300.0f, 600.0f)))
    {
        OutLocation = NavLocation.Location;
        return true;
    }

    OutLocation = Enemy->PatrolOrigin;
    return true;
}

void ASearchEscapeEnemyAIController::ClearTargetAndReturnToPatrol(ASearchEscapeEnemyCharacter* Enemy)
{
    CurrentTarget = nullptr;
    LastKnownTargetLocation = FVector::ZeroVector;
    bHasLineOfSight = false;
    LostSightElapsed = 0.0f;
    InvestigateElapsed = 0.0f;
    bHasPatrolLocation = false;
    PatrolWaitRemaining = 0.0f;
    MoveRefreshRemaining = 0.0f;

    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->ClearValue(TargetActorKey);
        BB->ClearValue(TargetLocationKey);
        BB->SetValueAsBool(HasLineOfSightKey, false);
    }

    if (Enemy && !Enemy->IsDead())
    {
        Enemy->SetEnemyState(ESearchEscapeEnemyState::Patrol);
    }
}

void ASearchEscapeEnemyAIController::SyncBlackboardTarget()
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!BB)
    {
        return;
    }

    if (CurrentTarget)
    {
        BB->SetValueAsObject(TargetActorKey, CurrentTarget);
    }
    else
    {
        BB->ClearValue(TargetActorKey);
    }

    BB->SetValueAsVector(TargetLocationKey, LastKnownTargetLocation);
    BB->SetValueAsVector(PatrolLocationKey, CurrentPatrolLocation);
    BB->SetValueAsBool(HasLineOfSightKey, bHasLineOfSight);
}
