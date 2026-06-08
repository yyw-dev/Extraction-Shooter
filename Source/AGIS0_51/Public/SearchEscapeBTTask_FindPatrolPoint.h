#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SearchEscapeBTTask_FindPatrolPoint.generated.h"

UCLASS()
class AGIS0_51_API USearchEscapeBTTask_FindPatrolPoint : public UBTTaskNode
{
    GENERATED_BODY()

public:
    USearchEscapeBTTask_FindPatrolPoint();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    FName PatrolLocationKey = TEXT("PatrolLocation");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float SearchRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float AcceptableRadius = 100.0f;

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
