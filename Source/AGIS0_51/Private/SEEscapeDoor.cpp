#include "SEEscapeDoor.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "SearchEscapePlayerComponent.h"
#include "SEGameMode.h"
#include "UObject/ConstructorHelpers.h"

ASEEscapeDoor::ASEEscapeDoor()
{
	PrimaryActorTick.bCanEverTick = true;

	EscapeBox = CreateDefaultSubobject<UBoxComponent>(TEXT("EscapeBox"));
	RootComponent = EscapeBox;
	EscapeBox->SetBoxExtent(FVector(120.0f, 50.0f, 170.0f));
	EscapeBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	EscapeBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	EscapeBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetRelativeLocation(FVector(0.0f, 0.0f, 85.0f));
	Mesh->SetRelativeScale3D(FVector(1.4f, 0.18f, 3.2f));
	Mesh->SetCollisionProfileName(TEXT("BlockAll"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}
}

void ASEEscapeDoor::BeginPlay()
{
	Super::BeginPlay();

	SE_SetActive(SE_IsActive);
}

void ASEEscapeDoor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!SE_IsActive)
	{
		return;
	}

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (!Player)
	{
		return;
	}

	// Check if player is dead via SE component
	USearchEscapePlayerComponent* SEComp = Player->FindComponentByClass<USearchEscapePlayerComponent>();
	if (SEComp && SEComp->SE_IsDead)
	{
		return;
	}

	const float Distance2D = FVector::Dist2D(Player->GetActorLocation(), GetActorLocation());
	const float HeightDelta = FMath::Abs(Player->GetActorLocation().Z - GetActorLocation().Z);
	if (Distance2D <= SE_InteractionRadius && HeightDelta <= 220.0f)
	{
		if (ASEGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ASEGameMode>() : nullptr)
		{
			GameMode->SE_EndGameEscape();
		}
	}
}

void ASEEscapeDoor::SE_SetActive(bool bNewActive)
{
	SE_IsActive = bNewActive;
	SetActorHiddenInGame(!SE_IsActive);
	SetActorEnableCollision(SE_IsActive);
	SetActorTickEnabled(true);
}
