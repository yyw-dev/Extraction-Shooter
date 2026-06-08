#include "SearchEscapeHealthComponent.h"

USearchEscapeHealthComponent::USearchEscapeHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USearchEscapeHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    bIsDead = false;
}

void USearchEscapeHealthComponent::TakeDamage(float Amount, AActor* Instigator)
{
    if (bIsDead || Amount <= 0.0f)
    {
        return;
    }

    const float PreviousHealth = CurrentHealth;
    CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.0f, MaxHealth);
    const float Delta = CurrentHealth - PreviousHealth;

    OnHealthChanged.Broadcast(GetOwner(), CurrentHealth, Delta);

    if (CurrentHealth <= 0.0f)
    {
        bIsDead = true;
        OnDeath.Broadcast(GetOwner(), Instigator);
    }
}

void USearchEscapeHealthComponent::Heal(float Amount)
{
    if (bIsDead || Amount <= 0.0f)
    {
        return;
    }

    const float PreviousHealth = CurrentHealth;
    CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, MaxHealth);
    const float Delta = CurrentHealth - PreviousHealth;

    OnHealthChanged.Broadcast(GetOwner(), CurrentHealth, Delta);
}

float USearchEscapeHealthComponent::GetHealthPercent() const
{
    return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

void USearchEscapeHealthComponent::ResetHealth()
{
    bIsDead = false;
    CurrentHealth = MaxHealth;
    OnHealthChanged.Broadcast(GetOwner(), CurrentHealth, MaxHealth);
}
