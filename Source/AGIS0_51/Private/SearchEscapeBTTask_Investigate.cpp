#include "SearchEscapeBTTask_Investigate.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

USearchEscapeBTTask_Investigate::USearchEscapeBTTask_Investigate()
{
    NodeName = TEXT("Investigate");
    bNotifyTick = true;
}

EBTNodeResult::Type USearchEscapeBTTask_Investigate::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    FBTInvestigateMemory* Memory = reinterpret_cast<FBTInvestigateMemory*>(NodeMemory);
    Memory->TimeWaited = 0.0f;

    AAIController* AIC = OwnerComp.GetAIOwner();
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!AIC || !BB)
    {
        return EBTNodeResult::Failed;
    }

    const FVector TargetLocation = BB->GetValueAsVector(TargetLocationKey);
    if (TargetLocation.IsNearlyZero())
    {
        return EBTNodeResult::Failed;
    }

    // Move to last known location
    AIC->MoveToLocation(TargetLocation, 100.0f);

    return EBTNodeResult::InProgress;
}

void USearchEscapeBTTask_Investigate::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    FBTInvestigateMemory* Memory = reinterpret_cast<FBTInvestigateMemory*>(NodeMemory);

    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // Check if we reached the location or waited long enough
    if (AIC->GetMoveStatus() == EPathFollowingStatus::Type::Idle)
    {
        Memory->TimeWaited += DeltaSeconds;
        if (Memory->TimeWaited >= WaitTime)
        {
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        }
    }
}
