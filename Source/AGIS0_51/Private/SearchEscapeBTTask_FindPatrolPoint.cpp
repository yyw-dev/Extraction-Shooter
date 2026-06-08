#include "SearchEscapeBTTask_FindPatrolPoint.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "SearchEscapeEnemyCharacter.h"

USearchEscapeBTTask_FindPatrolPoint::USearchEscapeBTTask_FindPatrolPoint()
{
    NodeName = TEXT("Find Patrol Point");
}

EBTNodeResult::Type USearchEscapeBTTask_FindPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC)
    {
        return EBTNodeResult::Failed;
    }

    APawn* MyPawn = AIC->GetPawn();
    if (!MyPawn)
    {
        return EBTNodeResult::Failed;
    }

    ASearchEscapeEnemyCharacter* Enemy = Cast<ASearchEscapeEnemyCharacter>(MyPawn);
    if (!Enemy)
    {
        return EBTNodeResult::Failed;
    }

    const FVector Origin = Enemy->PatrolOrigin;
    UBlackboardComponent* BB = const_cast<UBlackboardComponent*>(OwnerComp.GetBlackboardComponent());
    if (!BB)
    {
        return EBTNodeResult::Failed;
    }

    FVector PatrolPoint = Origin;
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLocation;
        const float Radius = FMath::Max(SearchRadius, Enemy->PatrolRadius);
        if (NavSys->GetRandomReachablePointInRadius(Origin, Radius, NavLocation))
        {
            PatrolPoint = NavLocation.Location;
        }
        else if (NavSys->ProjectPointToNavigation(Origin, NavLocation, FVector(300.0f, 300.0f, 600.0f)))
        {
            PatrolPoint = NavLocation.Location;
        }
    }

    BB->SetValueAsVector(PatrolLocationKey, PatrolPoint);
    return EBTNodeResult::Succeeded;
}
