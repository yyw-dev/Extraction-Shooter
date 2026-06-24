#include "SearchEscapePlayerComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SEVendor.h"

USearchEscapePlayerComponent::USearchEscapePlayerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USearchEscapePlayerComponent::BeginPlay()
{
	Super::BeginPlay();
	SE_Health = SE_MaxHealth;
	SE_Gold = SE_StartingGold;
}

// ---- Health ----
void USearchEscapePlayerComponent::SE_TakeDamage(float Amount)
{
	if (SE_IsDead || Amount <= 0.0f)
	{
		return;
	}

	SE_Health = FMath::Clamp(SE_Health - Amount, 0.0f, SE_MaxHealth);
	if (SE_Health <= 0.0f)
	{
		SE_IsDead = true;

		// Enable ragdoll
		if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
		{
			if (USkeletalMeshComponent* Mesh = Owner->GetMesh())
			{
				Mesh->SetAllBodiesSimulatePhysics(true);
				Mesh->SetSimulatePhysics(true);
				Mesh->SetCollisionProfileName(TEXT("Ragdoll"));
				Mesh->WakeAllRigidBodies();
			}
			if (UCapsuleComponent* Capsule = Owner->GetCapsuleComponent())
			{
				Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
			if (UCharacterMovementComponent* Movement = Owner->GetCharacterMovement())
			{
				Movement->DisableMovement();
			}
		}

		OnDeath.Broadcast();
	}
}

float USearchEscapePlayerComponent::GetHealthPercent() const
{
	return SE_MaxHealth > 0.0f ? SE_Health / SE_MaxHealth : 0.0f;
}

// ---- Gold ----
void USearchEscapePlayerComponent::SE_AddGold(int32 Amount)
{
	if (Amount <= 0) return;
	SE_Gold += Amount;
	OnGoldChanged(SE_Gold);
}

bool USearchEscapePlayerComponent::SE_SpendGold(int32 Amount)
{
	if (Amount <= 0) return true;
	if (SE_Gold < Amount) return false;
	SE_Gold -= Amount;
	OnGoldChanged(SE_Gold);
	return true;
}

// ---- Combat Power ----
void USearchEscapePlayerComponent::SE_AddCombatPower(int32 Amount)
{
	SE_CombatPower = FMath::Max(0, SE_CombatPower + Amount);
}

// ---- Ammo ----
int32 USearchEscapePlayerComponent::ReloadWeapon(int32 CurrentBulletCount)
{
	if (!CanReload(CurrentBulletCount)) return CurrentBulletCount;
	const int32 Needed = MaxMagazineAmmo - CurrentBulletCount;
	const int32 ToLoad = FMath::Min(Needed, ReserveAmmo);
	ReserveAmmo -= ToLoad;
	return CurrentBulletCount + ToLoad;
}

bool USearchEscapePlayerComponent::CanReload(int32 CurrentBulletCount) const
{
	return CurrentBulletCount < MaxMagazineAmmo && ReserveAmmo > 0;
}

void USearchEscapePlayerComponent::AddReserveAmmo(int32 Amount)
{
	ReserveAmmo = FMath::Max(0, ReserveAmmo + Amount);
}

int32 USearchEscapePlayerComponent::GetAmmoNeededToFill(int32 CurrentBulletCount) const
{
	return FMath::Max(0, MaxMagazineAmmo - CurrentBulletCount);
}

// ---- Interaction ----
void USearchEscapePlayerComponent::Interact()
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner || SE_IsDead) return;

	UCameraComponent* Camera = Owner->FindComponentByClass<UCameraComponent>();
	if (!Camera) return;

	const FVector Start = Camera->GetComponentLocation();
	const FVector End = Start + Camera->GetForwardVector() * InteractDistance;

	FCollisionShape Shape = FCollisionShape::MakeSphere(InteractRadius);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SEInteract), false, Owner);
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);

	TArray<FHitResult> Hits;
	if (GetWorld()->SweepMultiByObjectType(Hits, Start, End, FQuat::Identity, ObjectParams, Shape, Params))
	{
		for (const FHitResult& Hit : Hits)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && HitActor != Owner)
			{
				// Auto-handle vendor interaction
				if (ASEVendor* Vendor = Cast<ASEVendor>(HitActor))
				{
					Vendor->Interact(Owner);
				}
				OnInteractFound(HitActor);
				return;
			}
		}
	}
	OnInteractMiss();
}
