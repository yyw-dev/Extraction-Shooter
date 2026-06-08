#include "SearchEscapeDoor.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ASearchEscapeDoor::ASearchEscapeDoor()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    LeftPostMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftPostMesh"));
    LeftPostMesh->SetupAttachment(SceneRoot);
    LeftPostMesh->SetRelativeLocation(FVector(-160.0f, 0.0f, 250.0f));
    LeftPostMesh->SetRelativeScale3D(FVector(0.25f, 0.25f, 5.0f));
    LeftPostMesh->SetCollisionProfileName(TEXT("BlockAll"));

    RightPostMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightPostMesh"));
    RightPostMesh->SetupAttachment(SceneRoot);
    RightPostMesh->SetRelativeLocation(FVector(160.0f, 0.0f, 250.0f));
    RightPostMesh->SetRelativeScale3D(FVector(0.25f, 0.25f, 5.0f));
    RightPostMesh->SetCollisionProfileName(TEXT("BlockAll"));

    TopBeamMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TopBeamMesh"));
    TopBeamMesh->SetupAttachment(SceneRoot);
    TopBeamMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 515.0f));
    TopBeamMesh->SetRelativeScale3D(FVector(3.7f, 0.25f, 0.25f));
    TopBeamMesh->SetCollisionProfileName(TEXT("BlockAll"));

    EscapeTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("EscapeTrigger"));
    EscapeTrigger->SetupAttachment(SceneRoot);
    EscapeTrigger->SetRelativeLocation(FVector(0.0f, -120.0f, 160.0f));
    EscapeTrigger->SetBoxExtent(FVector(260.0f, 220.0f, 180.0f));
    EscapeTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    EscapeTrigger->SetCollisionObjectType(ECC_WorldDynamic);
    EscapeTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    EscapeTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMeshFinder.Succeeded())
    {
        LeftPostMesh->SetStaticMesh(CubeMeshFinder.Object);
        RightPostMesh->SetStaticMesh(CubeMeshFinder.Object);
        TopBeamMesh->SetStaticMesh(CubeMeshFinder.Object);
    }
}

void ASearchEscapeDoor::BeginPlay()
{
    Super::BeginPlay();

    EscapeTrigger->OnComponentBeginOverlap.AddDynamic(this, &ASearchEscapeDoor::OnEscapeTriggerBegin);
    SetDoorActive(bStartActive);

    if (bAutoActivateAfterDelay && !bStartActive)
    {
        if (ActivationDelay <= 0.0f)
        {
            ActivateDoor();
        }
        else
        {
            GetWorldTimerManager().SetTimer(ActivationTimerHandle, this, &ASearchEscapeDoor::ActivateDoor, ActivationDelay, false);
        }
    }
}

void ASearchEscapeDoor::ActivateDoor()
{
    bConsumed = false;
    SetDoorActive(true);
}

void ASearchEscapeDoor::DeactivateDoor()
{
    SetDoorActive(false);
}

void ASearchEscapeDoor::SetDoorActive(bool bNewActive)
{
    bIsActive = bNewActive;
    SetActorHiddenInGame(!bIsActive);
    EscapeTrigger->SetCollisionEnabled(bIsActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
    OnEscapeDoorStateChanged.Broadcast(this, bIsActive);

    if (bPrintDebugMessages)
    {
        const FString Message = bIsActive
            ? FString::Printf(TEXT("逃生门 %s 已激活"), *DoorId.ToString())
            : FString::Printf(TEXT("逃生门 %s 未激活"), *DoorId.ToString());
        UKismetSystemLibrary::PrintString(this, Message, true, true, bIsActive ? FLinearColor::Blue : FLinearColor::Gray, 2.0f);
    }
}

void ASearchEscapeDoor::CancelAutoActivation()
{
    bAutoActivateAfterDelay = false;

    if (GetWorld())
    {
        GetWorldTimerManager().ClearTimer(ActivationTimerHandle);
    }
}

void ASearchEscapeDoor::OnEscapeTriggerBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bIsActive || bConsumed || !OtherActor || OtherActor == this || !OtherActor->IsA<APawn>())
    {
        return;
    }

    if (bConsumeOnUse)
    {
        bConsumed = true;
    }

    if (bPrintDebugMessages)
    {
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("通过逃生门 %s 撤离成功"), *DoorId.ToString()), true, true, FLinearColor::Green, 4.0f);
    }

    OnEscapeSucceeded.Broadcast(this, OtherActor);
    ReceiveEscapeSucceeded(OtherActor);

    if (bConsumeOnUse)
    {
        DeactivateDoor();
    }
}
