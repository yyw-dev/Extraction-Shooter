#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SearchEscapeHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSearchEscapeHealthChangedSignature, AActor*, Owner, float, NewHealth, float, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSearchEscapeDeathSignature, AActor*, Owner, AActor*, Killer);

UCLASS(ClassGroup = (SearchEscape), meta = (BlueprintSpawnableComponent))
class AGIS0_51_API USearchEscapeHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USearchEscapeHealthComponent();

    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (ClampMin = "1.0"))
    float MaxHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float CurrentHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    bool bIsDead = false;

    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FSearchEscapeHealthChangedSignature OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FSearchEscapeDeathSignature OnDeath;

    UFUNCTION(BlueprintCallable, Category = "Health")
    void TakeDamage(float Amount, AActor* Instigator);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void Heal(float Amount);

    UFUNCTION(BlueprintPure, Category = "Health")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsDead() const { return bIsDead; }

    UFUNCTION(BlueprintCallable, Category = "Health")
    void ResetHealth();
};
