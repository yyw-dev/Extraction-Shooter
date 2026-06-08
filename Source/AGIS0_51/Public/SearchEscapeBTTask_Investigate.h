#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SearchEscapeBTTask_Investigate.generated.h"

UCLASS()
class AGIS0_51_API USearchEscapeBTTask_Investigate : public UBTTaskNode
{
    GENERATED_BODY()

public:
    USearchEscapeBTTask_Investigate();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    FName TargetLocationKey = TEXT("TargetLocation");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float WaitTime = 3.0f;

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTInvestigateMemory); }

private:
    struct FBTInvestigateMemory
    {
        float TimeWaited = 0.0f;
    };
};
