#include "SearchEscapeChest.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/ConstructorHelpers.h"

ASearchEscapeChest::ASearchEscapeChest()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
    BaseMesh->SetupAttachment(SceneRoot);
    BaseMesh->SetRelativeLocation(FVector(0.0, 0.0, 35.0));
    BaseMesh->SetRelativeScale3D(FVector(1.4, 0.9, 0.7));
    BaseMesh->SetCollisionProfileName(TEXT("BlockAll"));

    LidPivot = CreateDefaultSubobject<USceneComponent>(TEXT("LidPivot"));
    LidPivot->SetupAttachment(SceneRoot);
    LidPivot->SetRelativeLocation(FVector(0.0f, -47.5f, 82.0f));

    LidMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LidMesh"));
    LidMesh->SetupAttachment(LidPivot);
    LidMesh->SetRelativeLocation(FVector(0.0f, 47.5f, 0.0f));
    LidMesh->SetRelativeScale3D(FVector(1.45, 0.95, 0.18));
    LidMesh->SetCollisionProfileName(TEXT("BlockAll"));

    InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
    InteractionSphere->SetupAttachment(SceneRoot);
    InteractionSphere->InitSphereRadius(250.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionSphere->SetCollisionObjectType(ECC_WorldDynamic);
    InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMeshFinder.Succeeded())
    {
        BaseMesh->SetStaticMesh(CubeMeshFinder.Object);
        LidMesh->SetStaticMesh(CubeMeshFinder.Object);
    }
}

void ASearchEscapeChest::BeginPlay()
{
    Super::BeginPlay();

    InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ASearchEscapeChest::OnInteractionBegin);
    InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ASearchEscapeChest::OnInteractionEnd);
    SetOpenVisual(0.0f);
}

void ASearchEscapeChest::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bOpened || !IsValid(OpeningPlayer))
    {
        return;
    }

    const float SafeDuration = FMath::Max(OpenDuration, 0.1f);
    OpenProgress = FMath::Clamp(OpenProgress + DeltaSeconds / SafeDuration, 0.0f, 1.0f);
    SetOpenVisual(OpenProgress);
    OnChestProgressChanged.Broadcast(this, OpenProgress);

    if (OpenProgress >= 1.0f)
    {
        CompleteOpen();
    }
}

void ASearchEscapeChest::OnInteractionBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (bOpened || !OtherActor || OtherActor == this || !OtherActor->IsA<APawn>())
    {
        return;
    }

    OpeningPlayer = OtherActor;
    OpenProgress = 0.0f;
    SetOpenVisual(OpenProgress);

    if (bPrintDebugMessages)
    {
        UKismetSystemLibrary::PrintString(this, TEXT("正在开启宝箱，请停留 1 秒"), true, true, FLinearColor::Yellow, 1.0f);
    }
}

void ASearchEscapeChest::OnInteractionEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor && OtherActor == OpeningPlayer && !bOpened)
    {
        OpeningPlayer = nullptr;
        OpenProgress = 0.0f;
        SetOpenVisual(OpenProgress);
        OnChestProgressChanged.Broadcast(this, OpenProgress);

        if (bPrintDebugMessages)
        {
            UKismetSystemLibrary::PrintString(this, TEXT("离开范围，开启中断"), true, true, FLinearColor::Red, 1.0f);
        }
    }
}

void ASearchEscapeChest::SetOpenVisual(float Progress01)
{
    const float Angle = FMath::Lerp(0.0f, -75.0f, Progress01);
    LidPivot->SetRelativeRotation(FRotator(0.0f, 0.0f, Angle));
}

void ASearchEscapeChest::CompleteOpen()
{
    bOpened = true;
    OpenProgress = 1.0f;
    SetOpenVisual(1.0f);
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    const int32 Low = FMath::Min(MinGold, MaxGold);
    const int32 High = FMath::Max(MinGold, MaxGold);
    const int32 GoldAmount = FMath::RandRange(Low, High);

    if (bPrintDebugMessages)
    {
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("宝箱开启：+%d 金币"), GoldAmount), true, true, FLinearColor::Green, 2.0f);
    }

    OnChestOpened.Broadcast(this, OpeningPlayer, GoldAmount);
    ReceiveChestOpened(OpeningPlayer, GoldAmount);
    OpeningPlayer = nullptr;
}

void ASearchEscapeChest::ResetChest()
{
    bOpened = false;
    OpenProgress = 0.0f;
    OpeningPlayer = nullptr;
    InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SetOpenVisual(0.0f);
    OnChestProgressChanged.Broadcast(this, OpenProgress);
}

bool ASearchEscapeChest::IsPlayerOpening() const
{
    return !bOpened && IsValid(OpeningPlayer);
}
