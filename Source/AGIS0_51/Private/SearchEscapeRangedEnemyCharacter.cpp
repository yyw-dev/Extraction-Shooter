#include "SearchEscapeRangedEnemyCharacter.h"

#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NavigationSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "SearchEscapeArrowProjectile.h"
#include "SearchEscapeEnemyAIController.h"
#include "SearchEscapeHealthComponent.h"

ASearchEscapeRangedEnemyCharacter::ASearchEscapeRangedEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Wider attack range = shooting distance, AI will stop at range
	AttackRange = 2500.0f;

	// Archer never moves — zero speed always
	PatrolWalkSpeed = 0.0f;
	ChaseRunSpeed = 0.0f;

	AttackCooldown = 0.2f;
	RunAnimationSpeedThreshold = 200.0f;
	AttackDamage = 5.0f;
	AttackKnockbackStrength = 0.0f;
	AttackKnockbackUpStrength = 0.0f;

	// Archer stats
	HealthComponent->MaxHealth = 20.0f;
	HealthComponent->CurrentHealth = 20.0f;

	// Disable auto-rotation — archer manually faces target
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	bUseControllerRotationYaw = false;
}

void ASearchEscapeRangedEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	AttackRange = MaxShootRange;
	GetCharacterMovement()->MaxWalkSpeed = 0.0f;

	// Raise capsule height so perception origin is above waist-high railings
	GetCapsuleComponent()->SetCapsuleHalfHeight(160.0f);

	// Configure AI perception for tower sentry: long range, wide angle
	if (ASearchEscapeEnemyAIController* AIC = Cast<ASearchEscapeEnemyAIController>(GetController()))
	{
		AIC->SightRadius = MaxShootRange + 500.0f;
		AIC->LoseSightRadius = MaxShootRange + 1000.0f;
		AIC->PeripheralVisionAngleDegrees = 180.0f;

		// Reconfigure sight sense with new values
		if (AIC->EnemyPerceptionComponent && AIC->SightConfig)
		{
			AIC->SightConfig->SightRadius = AIC->SightRadius;
			AIC->SightConfig->LoseSightRadius = AIC->LoseSightRadius;
			AIC->SightConfig->PeripheralVisionAngleDegrees = 180.0f;
			AIC->EnemyPerceptionComponent->RequestStimuliListenerUpdate();
		}
	}
}

void ASearchEscapeRangedEnemyCharacter::ApplyMovementSettingsForState(ESearchEscapeEnemyState NewState)
{
	// Archer never moves regardless of state
	GetCharacterMovement()->MaxWalkSpeed = 0.0f;
}

void ASearchEscapeRangedEnemyCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Face the target while attacking or chasing
	if (CurrentState == ESearchEscapeEnemyState::Attack || CurrentState == ESearchEscapeEnemyState::Chase)
	{
		if (AAIController* AIC = Cast<AAIController>(GetController()))
		{
			if (AActor* Target = Cast<ASearchEscapeEnemyAIController>(AIC)->GetTargetActor())
			{
				const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
				if (ToTarget.SizeSquared2D() > 1.0f)
				{
					const FRotator TargetRot = FRotator(0.0f, ToTarget.Rotation().Yaw, 0.0f);
					SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaSeconds, 12.0f));
				}
			}
		}
	}
}

void ASearchEscapeRangedEnemyCharacter::PerformAttack(AActor* Target)
{
	if (!Target || IsDead())
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastShootTime < ShootInterval)
	{
		return;
	}
	LastShootTime = Now;

	// Face the target before shooting
	FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.0f;
	if (!ToTarget.IsNearlyZero())
	{
		SetActorRotation(ToTarget.Rotation());
	}

	PlayAttackAnimation();

	FireArrow(Target);
	ReceiveArrowFired(Target);

	if (bPrintDebugMessages)
	{
		UKismetSystemLibrary::PrintString(this,
			FString::Printf(TEXT("%s fired arrow at %s"), *GetName(), *Target->GetName()),
			true, false, FLinearColor(1.0f, 0.5f, 0.0f), 0.5f);
	}
}

void ASearchEscapeRangedEnemyCharacter::FireArrow(AActor* Target)
{
	if (!Target || !GetWorld())
	{
		return;
	}

	UClass* ProjectileClass = ArrowProjectileClass
		? ArrowProjectileClass.Get()
		: ASearchEscapeArrowProjectile::StaticClass();

	const FVector SpawnLocation = GetActorLocation()
		+ GetActorForwardVector() * ArrowSpawnOffset.X
		+ GetActorRightVector() * ArrowSpawnOffset.Y
		+ FVector(0.0f, 0.0f, ArrowSpawnOffset.Z);

	// Aim at target center
	FVector AimTarget = Target->GetActorLocation();
	AimTarget.Z += 100.0f;

	if (AimErrorDegrees > 0.0f)
	{
		const float ErrorAngle = FMath::FRandRange(-AimErrorDegrees, AimErrorDegrees);
		const FVector AimDirection = (AimTarget - SpawnLocation).GetSafeNormal();
		const FVector ScatteredAim = AimDirection.RotateAngleAxis(ErrorAngle, FVector::UpVector);
		AimTarget = SpawnLocation + ScatteredAim * 2000.0f;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASearchEscapeArrowProjectile* Arrow = GetWorld()->SpawnActor<ASearchEscapeArrowProjectile>(
		ProjectileClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

	if (Arrow)
	{
		Arrow->ArrowDamage = AttackDamage;
		Arrow->FireTowards(AimTarget, this);
	}
}
