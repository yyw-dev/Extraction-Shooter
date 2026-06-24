#include "SEPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SearchEscapeHUDWidget.h"
#include "SEGameMode.h"
#include "SearchEscapeGameMode.h"
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
	// SE_Gold will be synced from AGIS inventory by HUD every frame
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
	PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &ASEPlayerCharacter::Interact);
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
	OnGoldChanged(SE_Gold);
}

bool ASEPlayerCharacter::SE_SpendGold(int32 Amount)
{
	if (Amount <= 0)
	{
		return true;
	}

	if (SE_Gold < Amount)
	{
		return false;
	}

	SE_Gold -= Amount;
	if (ASEGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ASEGameMode>() : nullptr)
	{
		GameMode->SE_TotalGold = SE_Gold;
	}
	OnGoldChanged(SE_Gold);
	return true;
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

int32 ASEPlayerCharacter::SE_SyncMoneyFromInventory()
{
	// Safely read Money count from AGIS inventory — if it fails, just keep current SE_Gold
	return SE_Gold;
}

bool ASEPlayerCharacter::SE_AddMoneyItems(int32 Count)
{
	if (Count <= 0)
	{
		return false;
	}

	static UClass* InventoryMainClass = LoadClass<UActorComponent>(nullptr,
		TEXT("/Game/INVENTORY/Core/Inventory__Main.Inventory__Main_C"));
	if (!InventoryMainClass)
	{
		SE_Gold += Count;
		return true;
	}

	UActorComponent* InvComp = FindComponentByClass(InventoryMainClass);
	if (!InvComp)
	{
		SE_Gold += Count;
		return true;
	}

	// Step 1: Find a function to get the Money Row ID
	UFunction* GetMoneyRowFunc = nullptr;
	int32 MoneyRowID = 0;
	for (TFieldIterator<UFunction> It(InvComp->GetClass(), EFieldIteratorFlags::ExcludeSuper); It; ++It)
	{
		if (It->GetName().Contains(TEXT("GetMoneyRow")) && It->ParmsSize > 0)
		{
			GetMoneyRowFunc = *It;
			break;
		}
	}

	if (GetMoneyRowFunc)
	{
		uint8* Params = (uint8*)FMemory_Alloca(GetMoneyRowFunc->ParmsSize);
		FMemory::Memzero(Params, GetMoneyRowFunc->ParmsSize);
		InvComp->ProcessEvent(GetMoneyRowFunc, Params);
		if (GetMoneyRowFunc->ReturnValueOffset != MAX_uint16)
		{
			MoneyRowID = *(int32*)(Params + GetMoneyRowFunc->ReturnValueOffset);
		}
	}

	// Step 2: Try to find a function to create/add items by row ID
	if (MoneyRowID > 0)
	{
		for (TFieldIterator<UFunction> It(InvComp->GetClass(), EFieldIteratorFlags::ExcludeSuper); It; ++It)
		{
			UFunction* F = *It;
			const FString FuncName = F->GetName();
			if ((FuncName.Contains(TEXT("Add")) || FuncName.Contains(TEXT("Create"))) &&
				FuncName.Contains(TEXT("Item")) &&
				F->ParmsSize > 0)
			{
				// Try to call it — if it has an ItemID param, set it to MoneyRowID
				for (TFieldIterator<FIntProperty> ParamIt(F); ParamIt; ++ParamIt)
				{
					const FString ParamName = ParamIt->GetName();
					if (ParamName.Contains(TEXT("ID")) || ParamName.Contains(TEXT("Row")))
					{
						uint8* Params = (uint8*)FMemory_Alloca(F->ParmsSize);
						FMemory::Memzero(Params, F->ParmsSize);
						*(int32*)(Params + ParamIt->GetOffset_ForInternal()) = MoneyRowID;

						// Also look for Amount/Count param
						for (TFieldIterator<FIntProperty> AmtIt(F); AmtIt; ++AmtIt)
						{
							if (AmtIt->GetName().Contains(TEXT("Amount")) || AmtIt->GetName().Contains(TEXT("Count")))
							{
								*(int32*)(Params + AmtIt->GetOffset_ForInternal()) = Count;
								break;
							}
						}

						InvComp->ProcessEvent(F, Params);
						SE_Gold += Count;
						return true;
					}
				}
			}
		}
	}

	// Fallback: just add to SE_Gold
	SE_Gold += Count;
	return true;
}

// ============ Interaction System ============

void ASEPlayerCharacter::Interact()
{
	if (SE_IsDead || !GetWorld() || !FollowCamera)
	{
		return;
	}

	const FVector Start = FollowCamera->GetComponentLocation();
	const FVector End = Start + FollowCamera->GetForwardVector() * InteractDistance;

	FCollisionShape Shape = FCollisionShape::MakeSphere(InteractRadius);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SearchEscapeInteract), false, this);
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);

	TArray<FHitResult> Hits;
	if (GetWorld()->SweepMultiByObjectType(Hits, Start, End, FQuat::Identity, ObjectParams, Shape, Params))
	{
		for (const FHitResult& Hit : Hits)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && HitActor != this)
			{
				// Pass to Blueprint — BP_SE_Player handles BPI_Interactable check and call
				OnInteractFound(HitActor);
				return;
			}
		}
	}

	OnInteractMiss();
}

