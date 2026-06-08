#include "SEPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SEGameMode.h"
#include "SEMonster.h"
#include "SearchEscapeEnemyCharacter.h"
#include "SearchEscapeHealthComponent.h"
#include "UObject/ConstructorHelpers.h"

ASEPlayerCharacter::ASEPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 520.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 520.0f;
	GetCharacterMovement()->JumpZVelocity = 520.0f;
	GetCharacterMovement()->AirControl = 0.35f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 420.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 65.0f, 55.0f);
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/INVENTORY/Other/Demo/ThirdPerson/Characters/Mannequins/Meshes/SKM_Manny_FromAGIS.SKM_Manny_FromAGIS"));
	if (MeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshAsset.Object);
		GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -96.0f));
		GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	}

}

void ASEPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	SE_Health = SE_MaxHealth;
}

void ASEPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ASEPlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("Move Right / Left"), this, &ASEPlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("LookRight"), this, &ASEPlayerCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &ASEPlayerCharacter::LookUp);

	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ASEPlayerCharacter::StartJump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ASEPlayerCharacter::StopJump);
	PlayerInputComponent->BindAction(TEXT("SE_Attack"), IE_Pressed, this, &ASEPlayerCharacter::Attack);
}

void ASEPlayerCharacter::MoveForward(float Value)
{
	if (SE_IsDead || FMath::IsNearlyZero(Value) || !Controller)
	{
		return;
	}

	const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), Value);
}

void ASEPlayerCharacter::MoveRight(float Value)
{
	if (SE_IsDead || FMath::IsNearlyZero(Value) || !Controller)
	{
		return;
	}

	const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
	AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), Value);
}

void ASEPlayerCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ASEPlayerCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ASEPlayerCharacter::StartJump()
{
	if (!SE_IsDead)
	{
		Jump();
	}
}

void ASEPlayerCharacter::StopJump()
{
	StopJumping();
}

void ASEPlayerCharacter::Attack()
{
	if (SE_IsDead || !GetWorld())
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastAttackTime < SE_AttackCooldown)
	{
		return;
	}
	LastAttackTime = Now;

	const FVector Start = GetActorLocation() + FVector(0.0f, 0.0f, 55.0f);
	const FVector End = Start + GetActorForwardVector() * SE_AttackRange;

	FCollisionShape Shape = FCollisionShape::MakeSphere(SE_AttackRadius);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SearchEscapeAttack), false, this);
	TArray<FHitResult> Hits;
	GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ECC_Pawn, Shape, Params);

	if (bDebugAttackTrace)
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 0.4f, 0, 2.0f);
		DrawDebugSphere(GetWorld(), End, SE_AttackRadius, 16, FColor::Yellow, false, 0.4f);
	}

	TSet<AActor*> DamagedActors;
	const float Damage = SE_AttackDamage + static_cast<float>(SE_CombatPower) * 5.0f;
	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || DamagedActors.Contains(HitActor))
		{
			continue;
		}

		if (ASEMonster* Monster = Cast<ASEMonster>(HitActor))
		{
			DamagedActors.Add(HitActor);
			Monster->SE_TakeDamage(Damage, this);
		}
		else if (ASearchEscapeEnemyCharacter* Enemy = Cast<ASearchEscapeEnemyCharacter>(HitActor))
		{
			DamagedActors.Add(HitActor);
			FPointDamageEvent DamageEvent;
			Enemy->TakeDamage(Damage, DamageEvent, GetController(), this);
		}
	}
}

void ASEPlayerCharacter::SE_TakeDamage(float Amount)
{
	if (SE_IsDead || Amount <= 0.0f)
	{
		return;
	}

	SE_Health = FMath::Clamp(SE_Health - Amount, 0.0f, SE_MaxHealth);
	if (SE_Health <= 0.0f)
	{
		SE_IsDead = true;
		DisableInput(Cast<APlayerController>(GetController()));

		if (ASEGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ASEGameMode>() : nullptr)
		{
			GameMode->SE_EndGameDead();
		}
	}
}

void ASEPlayerCharacter::SE_AddGold(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	SE_Gold += Amount;
	if (ASEGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ASEGameMode>() : nullptr)
	{
		GameMode->SE_TotalGold = SE_Gold;
	}
}

void ASEPlayerCharacter::SE_AddCombatPower(int32 Amount)
{
	SE_CombatPower = FMath::Max(0, SE_CombatPower + Amount);
}

float ASEPlayerCharacter::GetHealthPercent() const
{
	return SE_MaxHealth > 0.0f ? SE_Health / SE_MaxHealth : 0.0f;
}

// ============ Simplified Ammo System ============

int32 ASEPlayerCharacter::ReloadWeapon(int32 CurrentBulletCount)
{
	if (!CanReload(CurrentBulletCount))
	{
		return CurrentBulletCount; // unchanged
	}

	const int32 Needed = MaxMagazineAmmo - CurrentBulletCount;
	const int32 ToLoad = FMath::Min(Needed, ReserveAmmo);

	ReserveAmmo -= ToLoad;
	return CurrentBulletCount + ToLoad;
}

bool ASEPlayerCharacter::CanReload(int32 CurrentBulletCount) const
{
	return CurrentBulletCount < MaxMagazineAmmo && ReserveAmmo > 0;
}

void ASEPlayerCharacter::AddReserveAmmo(int32 Amount)
{
	ReserveAmmo = FMath::Max(0, ReserveAmmo + Amount);
}

int32 ASEPlayerCharacter::GetAmmoNeededToFill(int32 CurrentBulletCount) const
{
	return FMath::Max(0, MaxMagazineAmmo - CurrentBulletCount);
}
