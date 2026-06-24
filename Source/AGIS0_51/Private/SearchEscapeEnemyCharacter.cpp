#include "SearchEscapeEnemyCharacter.h"

#include "AIController.h"
#include "Animation/AnimationAsset.h"
#include "Animation/AnimInstance.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NavigationSystem.h"
#include "SearchEscapeEnemyAIController.h"
#include "SearchEscapeEnemyHealthBarWidget.h"
#include "SearchEscapeHealthComponent.h"
#include "SearchEscapePlayerComponent.h"
#include "SEPlayerCharacter.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/ConstructorHelpers.h"

ASearchEscapeEnemyCharacter::ASearchEscapeEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    AIControllerClass = ASearchEscapeEnemyAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    bUseControllerRotationYaw = false;

    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCapsuleComponent()->SetCollisionObjectType(ECC_Pawn);
    // Block ALL trace channels so the AGIS rifle line trace (TraceTypeQuery1) always hits
    GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);

    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GetMesh()->SetCollisionObjectType(ECC_Pawn);
    // Block ALL trace channels on the mesh as well
    GetMesh()->SetCollisionResponseToAllChannels(ECR_Block);

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> DefaultEnemyMesh(TEXT("/Game/INVENTORY/Other/Demo/ThirdPerson/Characters/Mannequins/Meshes/SKM_Manny_FromAGIS.SKM_Manny_FromAGIS"));
    if (DefaultEnemyMesh.Succeeded())
    {
        GetMesh()->SetSkeletalMesh(DefaultEnemyMesh.Object);
        GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -96.0f));
        GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    }

    static ConstructorHelpers::FObjectFinder<UAnimationAsset> DefaultIdleAnimation(TEXT("/Game/INVENTORY/Other/Demo/ThirdPerson/Characters/Mannequins/Animations/Manny/MM_Idle.MM_Idle"));
    if (DefaultIdleAnimation.Succeeded())
    {
        IdleAnimation = DefaultIdleAnimation.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimationAsset> DefaultWalkAnimation(TEXT("/Game/INVENTORY/Other/Animations/WALK/MM_Unarmed_Walk_Fwd.MM_Unarmed_Walk_Fwd"));
    if (DefaultWalkAnimation.Succeeded())
    {
        WalkAnimation = DefaultWalkAnimation.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimationAsset> DefaultRunAnimation(TEXT("/Game/INVENTORY/Other/Animations/Jog/MM_Unarmed_Jog_Fwd.MM_Unarmed_Jog_Fwd"));
    if (DefaultRunAnimation.Succeeded())
    {
        RunAnimation = DefaultRunAnimation.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimationAsset> DefaultAttackAnimation(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Attack/MM_Attack_01.MM_Attack_01"));
    if (DefaultAttackAnimation.Succeeded())
    {
        AttackAnimation = DefaultAttackAnimation.Object;
    }

    HealthComponent = CreateDefaultSubobject<USearchEscapeHealthComponent>(TEXT("HealthComponent"));

    OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
    OverheadWidget->SetupAttachment(RootComponent);
    OverheadWidget->SetWidgetSpace(EWidgetSpace::World);
    OverheadWidget->SetWidgetClass(USearchEscapeEnemyHealthBarWidget::StaticClass());
    OverheadWidget->SetDrawSize(FVector2D(110.0f, 14.0f));
    OverheadWidget->SetPivot(FVector2D(0.5f, 0.5f));
    OverheadWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
    OverheadWidget->SetRelativeScale3D(FVector(0.45f));
    OverheadWidget->SetTwoSided(true);
    OverheadWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    OverheadWidget->SetVisibility(false);

    GetCharacterMovement()->MaxWalkSpeed = PatrolWalkSpeed;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
}

void ASearchEscapeEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    PatrolOrigin = GetActorLocation();
    InitialMeshRelativeTransform = GetMesh() ? GetMesh()->GetRelativeTransform() : FTransform::Identity;

    ConfigureDefaultMeshIfNeeded();
    ConfigureOverheadHealthBar();
    RestoreLocomotionAnimation();

    HealthComponent->OnDeath.AddDynamic(this, &ASearchEscapeEnemyCharacter::OnHealthDepleted);
    HealthComponent->OnHealthChanged.AddDynamic(this, &ASearchEscapeEnemyCharacter::OnHealthValueChanged);
    UpdateOverheadHealthBar();
    UpdateOverheadHealthBarVisibility();
}

void ASearchEscapeEnemyCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // AI movement and chase cancellation are handled by SearchEscapeEnemyAIController.
    UpdateLocomotionAnimation();
    FaceOverheadHealthBarToCamera();
}

float ASearchEscapeEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (DamageAmount <= 0.0f || IsDead() || !HealthComponent)
    {
        return 0.0f;
    }

    AActor* Killer = DamageCauser;
    if (!Killer && EventInstigator)
    {
        Killer = EventInstigator->GetPawn();
    }

    HealthComponent->TakeDamage(DamageAmount, Killer);
    ApplyKnockbackFrom(Killer, KnockbackStrength, KnockbackUpStrength);

    if (bPrintDebugMessages)
    {
        UKismetSystemLibrary::PrintString(this,
            FString::Printf(TEXT("[DEBUG] %s TakeDamage: %.0f from %s, Health=%.0f/%.0f"),
                *GetName(), DamageAmount,
                Killer ? *Killer->GetName() : TEXT("???"),
                HealthComponent->CurrentHealth, HealthComponent->MaxHealth),
            true, false, FLinearColor::Red, 2.0f);
    }

    return DamageAmount;
}

void ASearchEscapeEnemyCharacter::SE_TakeDamage(float Amount, ASEPlayerCharacter* InstigatorPlayer)
{
    if (Amount <= 0.0f || IsDead() || !HealthComponent)
    {
        return;
    }

    HealthComponent->TakeDamage(Amount, InstigatorPlayer);
    ApplyKnockbackFrom(InstigatorPlayer, KnockbackStrength, KnockbackUpStrength);

    if (bPrintDebugMessages)
    {
        UKismetSystemLibrary::PrintString(this,
            FString::Printf(TEXT("[DEBUG] %s SE_TakeDamage: %.0f from %s, Health=%.0f/%.0f"),
                *GetName(), Amount,
                InstigatorPlayer ? *InstigatorPlayer->GetName() : TEXT("???"),
                HealthComponent->CurrentHealth, HealthComponent->MaxHealth),
            true, false, FLinearColor::Red, 2.0f);
    }
}

void ASearchEscapeEnemyCharacter::SetEnemyState(ESearchEscapeEnemyState NewState)
{
    if (CurrentState == ESearchEscapeEnemyState::Dead || CurrentState == NewState)
    {
        return;
    }

    CurrentState = NewState;
    ApplyMovementSettingsForState(NewState);
    UpdateOverheadHealthBarVisibility();
    ReceiveEnemyStateChanged(NewState);

    if (bPrintDebugMessages)
    {
        const UEnum* EnumPtr = StaticEnum<ESearchEscapeEnemyState>();
        const FString StateName = EnumPtr ? EnumPtr->GetNameStringByValue(static_cast<int64>(NewState)) : TEXT("Unknown");
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s -> %s"), *GetName(), *StateName), true, false, FLinearColor::Yellow, 0.5f);
    }
}

void ASearchEscapeEnemyCharacter::ApplyMovementSettingsForState(ESearchEscapeEnemyState NewState)
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (!Movement)
    {
        return;
    }

    switch (NewState)
    {
    case ESearchEscapeEnemyState::Chase:
        Movement->MaxWalkSpeed = ChaseRunSpeed;
        break;
    case ESearchEscapeEnemyState::Attack:
        Movement->MaxWalkSpeed = 0.0f;
        break;
    case ESearchEscapeEnemyState::Dead:
        Movement->MaxWalkSpeed = 0.0f;
        break;
    case ESearchEscapeEnemyState::Investigate:
    case ESearchEscapeEnemyState::Patrol:
    default:
        Movement->MaxWalkSpeed = PatrolWalkSpeed;
        break;
    }
}

bool ASearchEscapeEnemyCharacter::CanAttackTarget(AActor* Target) const
{
    if (!bCanAttack || !Target || IsDead())
    {
        return false;
    }

    const float Dist = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    return Dist <= AttackRange;
}

void ASearchEscapeEnemyCharacter::PerformAttack(AActor* Target)
{
    if (!CanAttackTarget(Target))
    {
        return;
    }

    bCanAttack = false;
    PendingAttackTarget = Target;
    PlayAttackAnimation();

    ReceiveAttackPerformed(Target);

    if (bPrintDebugMessages)
    {
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s started attack on %s"), *GetName(), *Target->GetName()), true, false, FLinearColor::Red, 1.0f);
    }

    GetWorldTimerManager().ClearTimer(AttackImpactTimer);
    const float ImpactDelay = FMath::Clamp(AttackImpactDelay, 0.0f, AttackAnimationDuration);
    GetWorldTimerManager().SetTimer(AttackImpactTimer, this, &ASearchEscapeEnemyCharacter::ResolveAttackImpact, ImpactDelay, false);
    GetWorldTimerManager().SetTimer(AttackCooldownTimer, this, &ASearchEscapeEnemyCharacter::OnAttackCooldownEnd, FMath::Max(AttackCooldown, AttackAnimationDuration), false);
}

void ASearchEscapeEnemyCharacter::ApplyKnockbackFrom(AActor* Source, float HorizontalStrength, float VerticalStrength)
{
    if (!Source)
    {
        return;
    }

    FVector Direction = GetActorLocation() - Source->GetActorLocation();
    Direction.Z = 0.0f;
    if (!Direction.Normalize())
    {
        Direction = GetActorForwardVector() * -1.0f;
    }

    if (IsDead() && GetMesh() && GetMesh()->IsSimulatingPhysics())
    {
        GetMesh()->AddImpulse((Direction * HorizontalStrength) + FVector(0.0f, 0.0f, VerticalStrength), NAME_None, true);
        return;
    }

    if (AAIController* AIC = Cast<AAIController>(GetController()))
    {
        AIC->StopMovement();
    }

    LaunchCharacter((Direction * HorizontalStrength) + FVector(0.0f, 0.0f, VerticalStrength), true, true);
}

void ASearchEscapeEnemyCharacter::ResetEnemyForRun()
{
    GetWorldTimerManager().ClearTimer(AttackCooldownTimer);
    GetWorldTimerManager().ClearTimer(AttackAnimationTimer);
    GetWorldTimerManager().ClearTimer(AttackImpactTimer);
    PendingAttackTarget.Reset();
    bCanAttack = true;
    bIsPlayingAttackAnimation = false;
    CurrentLoopingAnimation = nullptr;

    if (GetMesh())
    {
        GetMesh()->SetSimulatePhysics(false);
        GetMesh()->SetAllBodiesSimulatePhysics(false);
        GetMesh()->bBlendPhysics = false;
        GetMesh()->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        GetMesh()->SetRelativeTransform(InitialMeshRelativeTransform);
        GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        GetMesh()->SetCollisionObjectType(ECC_Pawn);
        GetMesh()->SetCollisionResponseToAllChannels(ECR_Block);
    }

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
    GetCharacterMovement()->SetMovementMode(MOVE_Walking);

    if (HealthComponent)
    {
        HealthComponent->ResetHealth();
    }
    UpdateOverheadHealthBar();

    CurrentState = ESearchEscapeEnemyState::Patrol;
    ApplyMovementSettingsForState(CurrentState);
    UpdateOverheadHealthBarVisibility();
    PatrolOrigin = GetActorLocation();
    RestoreLocomotionAnimation();

    if (!GetController())
    {
        SpawnDefaultController();
    }
}

float ASearchEscapeEnemyCharacter::GetHealthPercent() const
{
    return HealthComponent ? HealthComponent->GetHealthPercent() : 0.0f;
}

bool ASearchEscapeEnemyCharacter::IsDead() const
{
    return HealthComponent ? HealthComponent->IsDead() : false;
}

void ASearchEscapeEnemyCharacter::OnHealthDepleted(AActor* InOwner, AActor* Killer)
{
    UpdateOverheadHealthBar();
    Die(Killer);
}

void ASearchEscapeEnemyCharacter::OnHealthValueChanged(AActor* InOwner, float NewHealth, float Delta)
{
    UpdateOverheadHealthBar();
}

void ASearchEscapeEnemyCharacter::OnAttackCooldownEnd()
{
    bCanAttack = true;
}

void ASearchEscapeEnemyCharacter::ConfigureDefaultMeshIfNeeded()
{
    if (!GetMesh() || GetMesh()->GetSkeletalMeshAsset())
    {
        return;
    }

    if (USkeletalMesh* DefaultEnemyMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/INVENTORY/Other/Demo/ThirdPerson/Characters/Mannequins/Meshes/SKM_Manny_FromAGIS.SKM_Manny_FromAGIS")))
    {
        GetMesh()->SetSkeletalMesh(DefaultEnemyMesh);
        GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -96.0f));
        GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
        InitialMeshRelativeTransform = GetMesh()->GetRelativeTransform();
    }
}

void ASearchEscapeEnemyCharacter::ConfigureOverheadHealthBar()
{
    if (!OverheadWidget)
    {
        return;
    }

    OverheadWidget->SetWidgetSpace(EWidgetSpace::World);
    OverheadWidget->SetWidgetClass(USearchEscapeEnemyHealthBarWidget::StaticClass());
    OverheadWidget->SetDrawSize(FVector2D(110.0f, 14.0f));
    OverheadWidget->SetPivot(FVector2D(0.5f, 0.5f));
    OverheadWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
    OverheadWidget->SetRelativeScale3D(FVector(0.45f));
    OverheadWidget->SetTwoSided(true);
    OverheadWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    OverheadWidget->InitWidget();
    UpdateOverheadHealthBarVisibility();
}

void ASearchEscapeEnemyCharacter::UpdateOverheadHealthBar()
{
    if (!OverheadWidget || !HealthComponent)
    {
        return;
    }

    OverheadWidget->InitWidget();
    if (USearchEscapeEnemyHealthBarWidget* HealthBarWidget = Cast<USearchEscapeEnemyHealthBarWidget>(OverheadWidget->GetUserWidgetObject()))
    {
        HealthBarWidget->SetHealthPercent(HealthComponent->GetHealthPercent());
    }
    UpdateOverheadHealthBarVisibility();
}

void ASearchEscapeEnemyCharacter::UpdateOverheadHealthBarVisibility()
{
    if (!OverheadWidget)
    {
        return;
    }

    const bool bShouldShow =
        !IsDead() &&
        (CurrentState == ESearchEscapeEnemyState::Chase || CurrentState == ESearchEscapeEnemyState::Attack);

    OverheadWidget->SetVisibility(bShouldShow);
    OverheadWidget->SetHiddenInGame(!bShouldShow);
}

void ASearchEscapeEnemyCharacter::FaceOverheadHealthBarToCamera()
{
    if (!OverheadWidget || !OverheadWidget->IsVisible())
    {
        return;
    }

    APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
    if (!CameraManager)
    {
        return;
    }

    const FRotator LookAtCamera = UKismetMathLibrary::FindLookAtRotation(
        OverheadWidget->GetComponentLocation(),
        CameraManager->GetCameraLocation()
    );
    OverheadWidget->SetWorldRotation(LookAtCamera);
}

void ASearchEscapeEnemyCharacter::UpdateLocomotionAnimation()
{
    if (IsDead() || bIsPlayingAttackAnimation || !GetMesh())
    {
        return;
    }

    UAnimationAsset* DesiredAnimation = IdleAnimation;
    const float Speed = GetVelocity().Size2D();
    if (Speed > 5.0f)
    {
        DesiredAnimation = (CurrentState == ESearchEscapeEnemyState::Chase || Speed >= RunAnimationSpeedThreshold) ? RunAnimation : WalkAnimation;
    }

    PlayLoopingAnimation(DesiredAnimation);
}

void ASearchEscapeEnemyCharacter::PlayLoopingAnimation(UAnimationAsset* Animation)
{
    if (!Animation || !GetMesh() || CurrentLoopingAnimation == Animation)
    {
        return;
    }

    GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    GetMesh()->PlayAnimation(Animation, true);
    CurrentLoopingAnimation = Animation;
}

void ASearchEscapeEnemyCharacter::PlayAttackAnimation()
{
    if (!AttackAnimation || !GetMesh() || IsDead())
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(AttackAnimationTimer);
    bIsPlayingAttackAnimation = true;
    CurrentLoopingAnimation = nullptr;
    GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    GetMesh()->PlayAnimation(AttackAnimation, false);
    GetWorldTimerManager().SetTimer(AttackAnimationTimer, this, &ASearchEscapeEnemyCharacter::RestoreLocomotionAnimation, AttackAnimationDuration, false);
}

void ASearchEscapeEnemyCharacter::ResolveAttackImpact()
{
    AActor* Target = PendingAttackTarget.Get();
    PendingAttackTarget.Reset();

    if (!IsTargetStillInAttackRange(Target))
    {
        if (bPrintDebugMessages)
        {
            UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s attack missed"), *GetName()), true, false, FLinearColor::Yellow, 0.8f);
        }
        return;
    }

    bool bDamageApplied = false;

    if (USearchEscapeHealthComponent* TargetHealth = Target->FindComponentByClass<USearchEscapeHealthComponent>())
    {
        TargetHealth->TakeDamage(AttackDamage, this);
        bDamageApplied = true;
    }
    else if (ASEPlayerCharacter* Player = Cast<ASEPlayerCharacter>(Target))
    {
        Player->SE_TakeDamage(AttackDamage);
        bDamageApplied = true;
    }
    else if (USearchEscapePlayerComponent* SEComp = Target->FindComponentByClass<USearchEscapePlayerComponent>())
    {
        SEComp->SE_TakeDamage(AttackDamage);
        bDamageApplied = true;
    }
    else
    {
        // Fallback: use standard UE damage pipeline
        Target->TakeDamage(AttackDamage, FDamageEvent(), GetController(), this);
        bDamageApplied = true;
    }
    ApplyKnockbackToTarget(Target);

    if (bPrintDebugMessages)
    {
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s hit %s for %.0f damage (applied=%d)"), *GetName(), Target ? *Target->GetName() : TEXT("null"), AttackDamage, bDamageApplied ? 1 : 0), true, false, FLinearColor::Red, 1.0f);
    }
}

bool ASearchEscapeEnemyCharacter::IsTargetStillInAttackRange(AActor* Target) const
{
    if (!Target || IsDead())
    {
        return false;
    }

    const float Dist = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    return Dist <= AttackRange;
}

void ASearchEscapeEnemyCharacter::ApplyKnockbackToTarget(AActor* Target) const
{
    ACharacter* TargetCharacter = Cast<ACharacter>(Target);
    if (!TargetCharacter)
    {
        return;
    }

    FVector Direction = TargetCharacter->GetActorLocation() - GetActorLocation();
    Direction.Z = 0.0f;
    if (!Direction.Normalize())
    {
        Direction = GetActorForwardVector();
    }

    TargetCharacter->LaunchCharacter((Direction * AttackKnockbackStrength) + FVector(0.0f, 0.0f, AttackKnockbackUpStrength), true, true);
}

void ASearchEscapeEnemyCharacter::RestoreLocomotionAnimation()
{
    if (!GetMesh() || IsDead())
    {
        return;
    }

    bIsPlayingAttackAnimation = false;
    CurrentLoopingAnimation = nullptr;
    UpdateLocomotionAnimation();
}

void ASearchEscapeEnemyCharacter::EnableRagdoll(AActor* Killer)
{
    if (!GetMesh())
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(AttackCooldownTimer);
    GetWorldTimerManager().ClearTimer(AttackAnimationTimer);
    GetWorldTimerManager().ClearTimer(AttackImpactTimer);
    PendingAttackTarget.Reset();
    bIsPlayingAttackAnimation = false;
    CurrentLoopingAnimation = nullptr;

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();
    if (OverheadWidget)
    {
        OverheadWidget->SetVisibility(false);
    }

    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetMesh()->SetCollisionObjectType(ECC_PhysicsBody);
    GetMesh()->SetCollisionResponseToAllChannels(ECR_Block);
    GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    GetMesh()->SetAllBodiesSimulatePhysics(true);
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->WakeAllRigidBodies();
    GetMesh()->bBlendPhysics = true;

    FVector ImpulseDirection = GetActorForwardVector();
    if (Killer)
    {
        ImpulseDirection = GetActorLocation() - Killer->GetActorLocation();
        ImpulseDirection.Z = 0.0f;
        if (!ImpulseDirection.Normalize())
        {
            ImpulseDirection = GetActorForwardVector();
        }
    }
    GetMesh()->AddImpulse((ImpulseDirection * DeathImpulseStrength) + FVector(0.0f, 0.0f, DeathImpulseStrength * 0.25f), NAME_None, true);
}

void ASearchEscapeEnemyCharacter::Die(AActor* Killer)
{
    SetEnemyState(ESearchEscapeEnemyState::Dead);
    ReceiveEnemyDied(Killer);

    // Stop AI
    if (AAIController* AIC = Cast<AAIController>(GetController()))
    {
        AIC->StopMovement();
        AIC->UnPossess();
    }

    EnableRagdoll(Killer);

    if (bPrintDebugMessages)
    {
        const FString KillerName = Killer ? Killer->GetName() : TEXT("Unknown");
        UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%s killed by %s"), *GetName(), *KillerName), true, true, FLinearColor::Red, 3.0f);
    }
}
