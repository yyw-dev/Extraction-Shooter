#include "SearchEscapeArrowProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SEPlayerCharacter.h"
#include "SearchEscapeEnemyCharacter.h"
#include "SearchEscapeHealthComponent.h"
#include "SEMonster.h"
#include "UObject/ConstructorHelpers.h"

ASearchEscapeArrowProjectile::ASearchEscapeArrowProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(20.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = CollisionSphere;

	// Visible arrow mesh — scaled-down cone so we can see it
	ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
	ArrowMesh->SetupAttachment(RootComponent);
	ArrowMesh->SetRelativeScale3D(FVector(0.15f, 0.15f, 1.2f));
	ArrowMesh->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f)); // cone +Z tip → local +X
	ArrowMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
	if (ConeMesh.Succeeded())
	{
		ArrowMesh->SetStaticMesh(ConeMesh.Object);
	}

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = ArrowSpeed;
	ProjectileMovement->MaxSpeed = ArrowSpeed;
	ProjectileMovement->ProjectileGravityScale = ArrowGravity;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
}

void ASearchEscapeArrowProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionSphere->OnComponentHit.AddDynamic(this, &ASearchEscapeArrowProjectile::OnArrowHit);
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ASearchEscapeArrowProjectile::OnArrowOverlap);

	SetLifeSpan(LifetimeSeconds);
}

void ASearchEscapeArrowProjectile::FireTowards(const FVector& TargetLocation, AActor* Shooter)
{
	OwningShooter = Shooter;

	const FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
	ProjectileMovement->Velocity = Direction * ArrowSpeed;
	SetActorRotation(Direction.Rotation());
}

void ASearchEscapeArrowProjectile::OnArrowHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this || OtherActor == OwningShooter)
	{
		return;
	}

	// Don't damage other enemies
	if (OtherActor->IsA<ASearchEscapeEnemyCharacter>() || OtherActor->IsA<ASEMonster>())
	{
		return;
	}

	ExpireArrow();
}

void ASearchEscapeArrowProjectile::OnArrowOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || OtherActor == OwningShooter)
	{
		return;
	}

	// Don't damage other enemies
	if (OtherActor->IsA<ASearchEscapeEnemyCharacter>() || OtherActor->IsA<ASEMonster>())
	{
		return;
	}

	// Damage the target
	if (USearchEscapeHealthComponent* TargetHealth = OtherActor->FindComponentByClass<USearchEscapeHealthComponent>())
	{
		TargetHealth->TakeDamage(ArrowDamage, OwningShooter.Get());
	}
	else if (ASEPlayerCharacter* Player = Cast<ASEPlayerCharacter>(OtherActor))
	{
		Player->SE_TakeDamage(ArrowDamage);
	}
	else
	{
		OtherActor->TakeDamage(ArrowDamage, FDamageEvent(), nullptr, OwningShooter.Get());
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red,
			FString::Printf(TEXT("Arrow hit %s: %.0f dmg"), *OtherActor->GetName(), ArrowDamage));
	}

	// Don't expire on pawn overlap — arrow passes through, can hit multiple targets
}

void ASearchEscapeArrowProjectile::ExpireArrow()
{
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMovement->StopMovementImmediately();
	SetLifeSpan(0.3f);
}
