#include "SEMonster.h"

#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "SEGameMode.h"
#include "SEPlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASEMonster::ASEMonster()
{
	PrimaryActorTick.bCanEverTick = true;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AAIController::StaticClass();

	GetCapsuleComponent()->InitCapsuleSize(44.0f, 88.0f);
	GetCharacterMovement()->MaxWalkSpeed = 285.0f;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 420.0f, 0.0f);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(RootComponent);
	VisualMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -20.0f));
	VisualMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.25f));
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
	if (ConeMesh.Succeeded())
	{
		VisualMesh->SetStaticMesh(ConeMesh.Object);
	}
}

void ASEMonster::BeginPlay()
{
	Super::BeginPlay();

	SE_Health = SE_MaxHealth;
	SpawnLocation = GetActorLocation();
	PickPatrolPoint();

	if (!GetController())
	{
		SpawnDefaultController();
	}
}

void ASEMonster::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateAI(DeltaSeconds);
}

void ASEMonster::UpdateAI(float DeltaSeconds)
{
	if (SE_IsDead)
	{
		return;
	}

	if (const ASEGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ASEGameMode>() : nullptr)
	{
		if (GameMode->SE_GameEnded)
		{
			return;
		}
	}

	ASEPlayerCharacter* Player = Cast<ASEPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player || Player->SE_IsDead)
	{
		Patrol();
		return;
	}

	const float DistanceToPlayer = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
	const float ChaseDistanceFromSpawn = FVector::Dist2D(SpawnLocation, Player->GetActorLocation());

	if (DistanceToPlayer <= SE_DetectRange && ChaseDistanceFromSpawn <= SE_ChaseLimit)
	{
		if (DistanceToPlayer <= SE_AttackRange)
		{
			AttackPlayer(Player);
		}
		else
		{
			ChasePlayer(Player);
		}
	}
	else
	{
		Patrol();
	}
}

void ASEMonster::Patrol()
{
	AAIController* AIController = Cast<AAIController>(GetController());
	if (!AIController)
	{
		return;
	}

	if (FVector::Dist2D(GetActorLocation(), PatrolTarget) < 120.0f)
	{
		PickPatrolPoint();
	}

	AIController->MoveToLocation(PatrolTarget, 70.0f, true, true, true, false);
}

void ASEMonster::ChasePlayer(ASEPlayerCharacter* Player)
{
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->MoveToActor(Player, SE_AttackRange * 0.75f, true, true, true);
	}
}

void ASEMonster::AttackPlayer(ASEPlayerCharacter* Player)
{
	if (!Player || !GetWorld())
	{
		return;
	}

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
	}

	const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	SetActorRotation(FRotator(0.0f, ToPlayer.Rotation().Yaw, 0.0f));

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastAttackTime >= AttackCooldown)
	{
		LastAttackTime = Now;
		Player->SE_TakeDamage(SE_Damage);
	}
}

void ASEMonster::SE_TakeDamage(float Amount, ASEPlayerCharacter* InstigatorPlayer)
{
	if (SE_IsDead || Amount <= 0.0f)
	{
		return;
	}

	SE_Health = FMath::Clamp(SE_Health - Amount, 0.0f, SE_MaxHealth);
	if (SE_Health <= 0.0f)
	{
		Die(InstigatorPlayer);
	}
}

void ASEMonster::Die(ASEPlayerCharacter* Killer)
{
	SE_IsDead = true;

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();
	VisualMesh->SetRelativeScale3D(FVector(1.15f, 1.15f, 0.18f));
	VisualMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -78.0f));

	if (Killer)
	{
		Killer->SE_AddGold(SE_RewardGold);
		Killer->SE_AddCombatPower(SE_RewardCombatPower);
	}

	SetLifeSpan(2.0f);
}

void ASEMonster::PickPatrolPoint()
{
	PatrolTarget = SpawnLocation;

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSystem)
	{
		return;
	}

	FNavLocation NavLocation;
	if (NavSystem->GetRandomReachablePointInRadius(SpawnLocation, SE_PatrolRadius, NavLocation))
	{
		PatrolTarget = NavLocation.Location;
	}
}
