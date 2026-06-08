#include "SEChest.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SEPlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASEChest::ASEChest()
{
	PrimaryActorTick.bCanEverTick = true;

	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	RootComponent = InteractionBox;
	InteractionBox->SetBoxExtent(FVector(90.0f, 90.0f, 65.0f));
	InteractionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetRelativeLocation(FVector(0.0f, 0.0f, -15.0f));
	Mesh->SetRelativeScale3D(FVector(1.25f, 0.85f, 0.55f));
	Mesh->SetCollisionProfileName(TEXT("BlockAll"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}
}

void ASEChest::BeginPlay()
{
	Super::BeginPlay();
}

void ASEChest::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (SE_IsOpened)
	{
		return;
	}

	ASEPlayerCharacter* Player = GetNearbyPlayer();
	if (!Player)
	{
		SE_OpenProgress = 0.0f;
		return;
	}

	SE_OpenProgress += DeltaSeconds;
	if (SE_OpenProgress >= SE_OpenDuration)
	{
		OpenChest(Player);
	}
}

ASEPlayerCharacter* ASEChest::GetNearbyPlayer() const
{
	ASEPlayerCharacter* Player = Cast<ASEPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (!Player || Player->SE_IsDead)
	{
		return nullptr;
	}

	const float Distance2D = FVector::Dist2D(Player->GetActorLocation(), GetActorLocation());
	const float HeightDelta = FMath::Abs(Player->GetActorLocation().Z - GetActorLocation().Z);
	if (Distance2D <= SE_InteractionRadius && HeightDelta <= 180.0f)
	{
		return Player;
	}

	return nullptr;
}

void ASEChest::OpenChest(ASEPlayerCharacter* Player)
{
	if (!Player || SE_IsOpened)
	{
		return;
	}

	SE_IsOpened = true;
	SE_OpenProgress = SE_OpenDuration;

	const int32 Reward = FMath::RandRange(SE_MinGold, SE_MaxGold);
	Player->SE_AddGold(Reward);

	Mesh->SetRelativeScale3D(FVector(1.25f, 0.85f, 0.18f));
	Mesh->SetRelativeLocation(FVector(0.0f, 0.0f, -55.0f));
	InteractionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
