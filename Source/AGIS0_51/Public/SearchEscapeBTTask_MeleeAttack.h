#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SearchEscapeBTTask_MeleeAttack.generated.h"

UCLASS()
class AGIS0_51_API USearchEscapeBTTask_MeleeAttack : public UBTTaskNode
{
    GENERATED_BODY()

public:
    USearchEscapeBTTask_MeleeAttack();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    FName TargetActorKey = TEXT("TargetActor");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    FName EnemyStateKey = TEXT("EnemyState");

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMeleeAttackMemory); }

private:
    struct FBTMeleeAttackMemory
    {
        AActor* CachedTarget = nullptr;
        float TimeSinceLastAttack = 0.0f;
    };
};
