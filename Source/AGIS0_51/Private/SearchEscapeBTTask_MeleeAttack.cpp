#include "SearchEscapeBTTask_MeleeAttack.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SearchEscapeEnemyCharacter.h"
#include "SearchEscapeHealthComponent.h"

USearchEscapeBTTask_MeleeAttack::USearchEscapeBTTask_MeleeAttack()
{
    NodeName = TEXT("Melee Attack");
    bNotifyTick = true;
}

EBTNodeResult::Type USearchEscapeBTTask_MeleeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    FBTMeleeAttackMemory* Memory = reinterpret_cast<FBTMeleeAttackMemory*>(NodeMemory);
    Memory->CachedTarget = nullptr;
    Memory->TimeSinceLastAttack = 0.0f;

    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC)
    {
        return EBTNodeResult::Failed;
    }

    ASearchEscapeEnemyCharacter* Enemy = Cast<ASearchEscapeEnemyCharacter>(AIC->GetPawn());
    if (!Enemy || Enemy->IsDead())
    {
        return EBTNodeResult::Failed;
    }

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    AActor* Target = BB ? Cast<AActor>(BB->GetValueAsObject(TargetActorKey)) : nullptr;
    if (!Target)
    {
        return EBTNodeResult::Failed;
    }

    Memory->CachedTarget = Target;

    if (Enemy->CanAttackTarget(Target))
    {
        Enemy->PerformAttack(Target);
        Memory->TimeSinceLastAttack = 0.0f;

        // Update blackboard state
        BB->SetValueAsEnum(EnemyStateKey, static_cast<uint8>(Enemy->CurrentState));
    }

    return EBTNodeResult::InProgress;
}

void USearchEscapeBTTask_MeleeAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    FBTMeleeAttackMemory* Memory = reinterpret_cast<FBTMeleeAttackMemory*>(NodeMemory);

    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    ASearchEscapeEnemyCharacter* Enemy = Cast<ASearchEscapeEnemyCharacter>(AIC->GetPawn());
    if (!Enemy || Enemy->IsDead())
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    AActor* Target = Memory->CachedTarget;
    if (!Target || !Target->IsValidLowLevel())
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // Check if target is dead
    USearchEscapeHealthComponent* TargetHealth = Target->FindComponentByClass<USearchEscapeHealthComponent>();
    if (TargetHealth && TargetHealth->IsDead())
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    Memory->TimeSinceLastAttack += DeltaSeconds;

    // Attack again after cooldown
    if (Memory->TimeSinceLastAttack >= Enemy->AttackCooldown)
    {
        if (Enemy->CanAttackTarget(Target))
        {
            Enemy->PerformAttack(Target);
            Memory->TimeSinceLastAttack = 0.0f;
        }
        else
        {
            // Target moved out of range, but keep chasing (BT selector will handle this)
        }
    }

    // Update blackboard location so we keep chasing
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsVector(TEXT("TargetLocation"), Target->GetActorLocation());
        BB->SetValueAsEnum(EnemyStateKey, static_cast<uint8>(Enemy->CurrentState));
    }
}
