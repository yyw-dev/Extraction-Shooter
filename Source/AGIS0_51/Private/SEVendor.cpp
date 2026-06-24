#include "SEVendor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "SearchEscapePlayerComponent.h"
#include "UObject/ConstructorHelpers.h"

ASEVendor::ASEVendor()
{
	PrimaryActorTick.bCanEverTick = true;

	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	InteractionBox->SetBoxExtent(FVector(150.0f, 150.0f, 150.0f));
	InteractionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionBox->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = InteractionBox;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
	Mesh->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}
}

void ASEVendor::BeginPlay()
{
	Super::BeginPlay();
	InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &ASEVendor::OnBoxBeginOverlap);
	InteractionBox->OnComponentEndOverlap.AddDynamic(this, &ASEVendor::OnBoxEndOverlap);
}

void ASEVendor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ASEVendor::OnBoxBeginOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (OtherActor && !PlayersInRange.Contains(OtherActor))
	{
		PlayersInRange.Add(OtherActor);
		OnPlayerEnterRange(OtherActor);
	}
}

void ASEVendor::OnBoxEndOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32)
{
	if (OtherActor && PlayersInRange.Contains(OtherActor))
	{
		PlayersInRange.Remove(OtherActor);
		OnPlayerLeaveRange(OtherActor);
	}
}

bool ASEVendor::CanPlayerAfford(AActor* Buyer, int32 ItemIndex) const
{
	if (!ShopItems.IsValidIndex(ItemIndex) || !Buyer) return false;

	USearchEscapePlayerComponent* SEComp = Buyer->FindComponentByClass<USearchEscapePlayerComponent>();
	if (!SEComp) return false;

	return SEComp->SE_Gold >= ShopItems[ItemIndex].Price;
}

bool ASEVendor::BuyItem(int32 ItemIndex, AActor* Buyer)
{
	if (!ShopItems.IsValidIndex(ItemIndex) || !Buyer) return false;

	USearchEscapePlayerComponent* SEComp = Buyer->FindComponentByClass<USearchEscapePlayerComponent>();
	if (!SEComp)
	{
		OnPurchaseFailed(TEXT("No SE Player Component"), Buyer);
		return false;
	}

	const FSEVendorItem& Item = ShopItems[ItemIndex];

	if (!SEComp->SE_SpendGold(Item.Price))
	{
		OnPurchaseFailed(FString::Printf(TEXT("Not enough gold for %s"), *Item.ItemName), Buyer);
		return false;
	}

	OnItemPurchased(ItemIndex, Buyer);
	return true;
}

void ASEVendor::Interact(AActor* Player)
{
	if (Player)
	{
		OnShopOpened(Player);
	}
}

FSEVendorItem ASEVendor::GetItem(int32 Index) const
{
	if (ShopItems.IsValidIndex(Index))
	{
		return ShopItems[Index];
	}
	return FSEVendorItem();
}
