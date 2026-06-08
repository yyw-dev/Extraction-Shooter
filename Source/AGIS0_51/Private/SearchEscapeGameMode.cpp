#include "SearchEscapeGameMode.h"

#include "Animation/AnimationAsset.h"
#include "Animation/AnimInstance.h"
#include "Blueprint/UserWidget.h"
#include "Components/SkeletalMeshComponent.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "Kismet/GameplayStatics.h"
#include "SearchEscapeChest.h"
#include "SearchEscapeDoor.h"
#include "SearchEscapeEnemyCharacter.h"
#include "SearchEscapeHealthComponent.h"
#include "SearchEscapeHUDWidget.h"
#include "UObject/ConstructorHelpers.h"

ASearchEscapeGameMode::ASearchEscapeGameMode()
{
    PrimaryActorTick.bCanEverTick = true;

    HUDWidgetClass = USearchEscapeHUDWidget::StaticClass();

}

void ASearchEscapeGameMode::BeginPlay()
{
    Super::BeginPlay();

    CreateHUD();
    SpawnActorsFromMarkers();
    BindGameplayActors();
    TimeRemaining = RoundDuration;
    PlayerCurrentHealth = PlayerMaxHealth;

    if (bShowStartScreen)
    {
        RunState = ESearchEscapeRunState::WaitingToStart;
        UpdateHUDState();
        if (HUDWidget)
        {
            SetInputForMenu(true);
        }
    }
    else
    {
        StartSearchEscapeGame();
    }
}

void ASearchEscapeGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!HUDWidget)
    {
        CreateHUD();
        UpdateHUDState();
    }

    if (RunState == ESearchEscapeRunState::WaitingToStart)
    {
        if (HUDWidget && !bMenuInputApplied)
        {
            SetInputForMenu(true);
        }
        return;
    }

    if (RunState != ESearchEscapeRunState::Playing)
    {
        return;
    }

    TimeRemaining = FMath::Max(0.0f, TimeRemaining - DeltaSeconds);
    OnTimeChanged.Broadcast(TimeRemaining);
    if (HUDWidget)
    {
        HUDWidget->SetTimeRemaining(TimeRemaining);
    }

    const float Elapsed = RoundDuration - TimeRemaining;
    if (!bEscapeDoorsActivated && Elapsed >= EscapeDoorActivationTime)
    {
        ActivateEscapeDoors();
    }

    if (TimeRemaining <= 0.0f)
    {
        EndSearchEscapeGame(false, FText::FromString(TEXT("倒计时结束，未能撤离")));
    }
}

void ASearchEscapeGameMode::CreateHUD()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
    {
        return;
    }

    TSubclassOf<USearchEscapeHUDWidget> WidgetClass = HUDWidgetClass;
    if (!WidgetClass)
    {
        WidgetClass = USearchEscapeHUDWidget::StaticClass();
    }
    HUDWidget = CreateWidget<USearchEscapeHUDWidget>(PC, WidgetClass);
    if (HUDWidget)
    {
        HUDWidget->SetGameMode(this);
        HUDWidget->AddToViewport(10);
        HUDWidget->SetGold(CurrentGold);
        HUDWidget->SetTimeRemaining(TimeRemaining);
        HUDWidget->SetKillCount(KillCount);
        HUDWidget->SetCombatScore(CombatScore);
        HUDWidget->SetPlayerHealth(PlayerCurrentHealth, PlayerMaxHealth);
    }
}

void ASearchEscapeGameMode::SpawnActorsFromMarkers()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    TArray<AActor*> ChestMarkers;
    UGameplayStatics::GetAllActorsWithTag(World, TEXT("SE.ChestMarker"), ChestMarkers);
    for (AActor* Marker : ChestMarkers)
    {
        if (!Marker || Marker->IsA<ASearchEscapeChest>())
        {
            continue;
        }

        World->SpawnActor<ASearchEscapeChest>(ASearchEscapeChest::StaticClass(), Marker->GetActorTransform());
        Marker->SetActorHiddenInGame(true);
        Marker->SetActorEnableCollision(false);
    }

    TArray<AActor*> MonsterMarkers;
    UGameplayStatics::GetAllActorsWithTag(World, TEXT("SE.MonsterSpawn"), MonsterMarkers);
    for (AActor* Marker : MonsterMarkers)
    {
        if (!Marker || Marker->IsA<ASearchEscapeEnemyCharacter>())
        {
            continue;
        }

        World->SpawnActor<ASearchEscapeEnemyCharacter>(ASearchEscapeEnemyCharacter::StaticClass(), Marker->GetActorTransform());
        Marker->SetActorHiddenInGame(true);
        Marker->SetActorEnableCollision(false);
    }

    TArray<AActor*> EscapeMarkers;
    UGameplayStatics::GetAllActorsWithTag(World, TEXT("SE.EscapePoint"), EscapeMarkers);
    for (int32 Index = 0; Index < EscapeMarkers.Num(); ++Index)
    {
        AActor* Marker = EscapeMarkers[Index];
        if (!Marker || Marker->IsA<ASearchEscapeDoor>())
        {
            continue;
        }

        ASearchEscapeDoor* Door = World->SpawnActor<ASearchEscapeDoor>(ASearchEscapeDoor::StaticClass(), Marker->GetActorTransform());
        if (Door)
        {
            Door->DoorId = FName(*FString::Printf(TEXT("%d"), Index + 1));
            Door->ActivationDelay = EscapeDoorActivationTime;
            Door->bAutoActivateAfterDelay = false;
            Door->bStartActive = false;
            Door->DeactivateDoor();
        }

        Marker->SetActorHiddenInGame(true);
        Marker->SetActorEnableCollision(false);
    }
}

void ASearchEscapeGameMode::SetInputForMenu(bool bMenuInput)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
    {
        return;
    }

    PC->SetShowMouseCursor(bMenuInput);
    bMenuInputApplied = bMenuInput;

    if (bMenuInput)
    {
        FInputModeUIOnly InputMode;
        if (HUDWidget)
        {
            InputMode.SetWidgetToFocus(HUDWidget->TakeWidget());
        }
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->SetIgnoreMoveInput(true);
        PC->SetIgnoreLookInput(true);
    }
    else
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->ResetIgnoreMoveInput();
        PC->ResetIgnoreLookInput();
    }
}

void ASearchEscapeGameMode::SetupPlayerAttackInput(APlayerController* PC)
{
    (void)PC;
    // AGIS owns weapon/fire input on BP_SE_Player. This fallback is intentionally left unbound.
}

void ASearchEscapeGameMode::RemovePlayerAttackInput(APlayerController* PC)
{
    (void)PC;
    // Do not clear PlayerController bindings; AGIS input must stay intact.
}

void ASearchEscapeGameMode::StartSearchEscapeGame()
{
    CurrentGold = 0;
    KillCount = 0;
    CombatScore = 0;
    TimeRemaining = RoundDuration;
    PlayerCurrentHealth = PlayerMaxHealth;
    RunState = ESearchEscapeRunState::Playing;
    bEscapeDoorsActivated = false;
    bPlayerCanAttack = true;
    bPlayerMovementLockedForAttack = false;
    GetWorldTimerManager().ClearTimer(PlayerAttackCooldownTimer);
    GetWorldTimerManager().ClearTimer(PlayerAttackAnimationTimer);
    GetWorldTimerManager().ClearTimer(PlayerAttackImpactTimer);
    GetWorldTimerManager().ClearTimer(PlayerAttackMovementLockTimer);

    SetupPlayerHealth();
    PrepareGameplayActorsForNewRun();
    BindEnemyDeathEvents();

    OnGoldChanged.Broadcast(CurrentGold);
    OnTimeChanged.Broadcast(TimeRemaining);
    OnKillCountChanged.Broadcast(KillCount);
    OnCombatScoreChanged.Broadcast(CombatScore);
    OnPlayerHealthChanged.Broadcast(PlayerCurrentHealth, PlayerMaxHealth);

    UpdateHUDState();
    SetInputForMenu(false);
}

void ASearchEscapeGameMode::SetupPlayerHealth()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!PlayerPawn)
    {
        return;
    }

    // Find or add health component to the player
    PlayerHealthComponent = PlayerPawn->FindComponentByClass<USearchEscapeHealthComponent>();
    if (!PlayerHealthComponent)
    {
        PlayerHealthComponent = Cast<USearchEscapeHealthComponent>(
            PlayerPawn->AddComponentByClass(USearchEscapeHealthComponent::StaticClass(), false, FTransform::Identity, false)
        );
    }

    PlayerHealthComponent->MaxHealth = PlayerMaxHealth;
    PlayerHealthComponent->OnDeath.RemoveDynamic(this, &ASearchEscapeGameMode::HandlePlayerHealthDepleted);
    PlayerHealthComponent->OnDeath.AddDynamic(this, &ASearchEscapeGameMode::HandlePlayerHealthDepleted);
    PlayerHealthComponent->OnHealthChanged.RemoveDynamic(this, &ASearchEscapeGameMode::HandlePlayerHealthChanged);
    PlayerHealthComponent->OnHealthChanged.AddDynamic(this, &ASearchEscapeGameMode::HandlePlayerHealthChanged);
    PlayerHealthComponent->ResetHealth();
    HandlePlayerHealthChanged(PlayerPawn, PlayerHealthComponent->CurrentHealth, 0.0f);
}

void ASearchEscapeGameMode::AddGold(int32 GoldAmount)
{
    if (GoldAmount <= 0 || RunState != ESearchEscapeRunState::Playing)
    {
        return;
    }

    CurrentGold += GoldAmount;
    OnGoldChanged.Broadcast(CurrentGold);

    if (HUDWidget)
    {
        HUDWidget->SetGold(CurrentGold);
    }
}

void ASearchEscapeGameMode::EndSearchEscapeGame(bool bSuccess, const FText& Reason)
{
    if (RunState != ESearchEscapeRunState::Playing)
    {
        return;
    }

    RunState = bSuccess ? ESearchEscapeRunState::Escaped : ESearchEscapeRunState::Failed;
    OnGameEnded.Broadcast(bSuccess, CurrentGold, Reason);
    UnlockPlayerFromAttack();

    if (HUDWidget)
    {
        HUDWidget->ShowEndScreen(bSuccess, CurrentGold, Reason);
    }

    SetInputForMenu(true);
}

void ASearchEscapeGameMode::RestartSearchEscapeGame()
{
    const FName CurrentLevel = FName(*UGameplayStatics::GetCurrentLevelName(this, true));
    UGameplayStatics::OpenLevel(this, CurrentLevel);
}

bool ASearchEscapeGameMode::IsGamePlaying() const
{
    return RunState == ESearchEscapeRunState::Playing;
}

void ASearchEscapeGameMode::PlayerAttack()
{
    if (!bPlayerCanAttack || RunState != ESearchEscapeRunState::Playing)
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!PlayerPawn)
    {
        return;
    }

    bPlayerCanAttack = false;
    LockPlayerForAttack();
    PlayPlayerAttackAnimation();
    GetWorldTimerManager().ClearTimer(PlayerAttackImpactTimer);
    const float ImpactDelay = FMath::Clamp(PlayerAttackImpactDelay, 0.0f, PlayerAttackAnimationDuration);
    GetWorldTimerManager().SetTimer(PlayerAttackImpactTimer, this, &ASearchEscapeGameMode::ResolvePlayerAttackImpact, ImpactDelay, false);
    GetWorldTimerManager().SetTimer(PlayerAttackCooldownTimer, this, &ASearchEscapeGameMode::OnPlayerAttackCooldownEnd, FMath::Max(PlayerAttackCooldown, PlayerAttackAnimationDuration), false);
    GetWorldTimerManager().ClearTimer(PlayerAttackMovementLockTimer);
    GetWorldTimerManager().SetTimer(PlayerAttackMovementLockTimer, this, &ASearchEscapeGameMode::UnlockPlayerFromAttack, PlayerAttackAnimationDuration, false);
}

void ASearchEscapeGameMode::ResolvePlayerAttackImpact()
{
    if (RunState != ESearchEscapeRunState::Playing)
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!PlayerPawn)
    {
        return;
    }

    // Find the closest enemy within attack range
    const FVector PlayerLocation = PlayerPawn->GetActorLocation();
    ASearchEscapeEnemyCharacter* ClosestEnemy = nullptr;
    float ClosestDist = PlayerAttackRange;

    for (TActorIterator<ASearchEscapeEnemyCharacter> It(GetWorld()); It; ++It)
    {
        ASearchEscapeEnemyCharacter* Enemy = *It;
        if (Enemy->IsDead())
        {
            continue;
        }

        const float Dist = FVector::Dist(PlayerLocation, Enemy->GetActorLocation());
        if (Dist <= ClosestDist)
        {
            ClosestDist = Dist;
            ClosestEnemy = Enemy;
        }
    }

    if (ClosestEnemy && ClosestEnemy->HealthComponent)
    {
        ClosestEnemy->HealthComponent->TakeDamage(PlayerAttackDamage, PlayerPawn);
        ClosestEnemy->ApplyKnockbackFrom(PlayerPawn, PlayerHitKnockbackStrength, PlayerHitKnockbackUpStrength);

        // Visual feedback
        UKismetSystemLibrary::PrintString(this,
            FString::Printf(TEXT("攻击命中 %s，造成 %.0f 点伤害"), *ClosestEnemy->GetName(), PlayerAttackDamage),
            true, false, FLinearColor(1.0f, 0.8f, 0.2f, 1.0f), 1.0f);
    }
}

void ASearchEscapeGameMode::LockPlayerForAttack()
{
    ACharacter* PlayerCharacter = GetPlayerCharacter();
    if (!PlayerCharacter || bPlayerMovementLockedForAttack)
    {
        return;
    }

    bPlayerMovementLockedForAttack = true;
    PlayerCharacter->StopJumping();

    if (UCharacterMovementComponent* Movement = PlayerCharacter->GetCharacterMovement())
    {
        Movement->StopMovementImmediately();
        Movement->DisableMovement();
    }

    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        PC->SetIgnoreMoveInput(true);
    }
}

void ASearchEscapeGameMode::UnlockPlayerFromAttack()
{
    ACharacter* PlayerCharacter = GetPlayerCharacter();
    bPlayerMovementLockedForAttack = false;

    if (PlayerCharacter && RunState == ESearchEscapeRunState::Playing)
    {
        if (UCharacterMovementComponent* Movement = PlayerCharacter->GetCharacterMovement())
        {
            Movement->SetMovementMode(MOVE_Walking);
        }
    }

    if (RunState == ESearchEscapeRunState::Playing)
    {
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
        {
            PC->ResetIgnoreMoveInput();
        }
    }
}

void ASearchEscapeGameMode::OnPlayerAttackCooldownEnd()
{
    bPlayerCanAttack = true;
}

void ASearchEscapeGameMode::ConfigurePlayerAnimation()
{
    ACharacter* PlayerCharacter = GetPlayerCharacter();
    if (!PlayerCharacter || !CharacterLocomotionAnimClass)
    {
        return;
    }

    if (USkeletalMeshComponent* Mesh = PlayerCharacter->GetMesh())
    {
        Mesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
        Mesh->SetAnimInstanceClass(CharacterLocomotionAnimClass);
    }
}

void ASearchEscapeGameMode::PlayPlayerAttackAnimation()
{
    ACharacter* PlayerCharacter = GetPlayerCharacter();
    if (!PlayerCharacter || !PlayerAttackAnimation)
    {
        return;
    }

    USkeletalMeshComponent* Mesh = PlayerCharacter->GetMesh();
    if (!Mesh)
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(PlayerAttackAnimationTimer);
    Mesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    Mesh->PlayAnimation(PlayerAttackAnimation, false);
    GetWorldTimerManager().SetTimer(PlayerAttackAnimationTimer, this, &ASearchEscapeGameMode::RestorePlayerLocomotionAnimation, PlayerAttackAnimationDuration, false);
}

void ASearchEscapeGameMode::RestorePlayerLocomotionAnimation()
{
    ConfigurePlayerAnimation();
}

ACharacter* ASearchEscapeGameMode::GetPlayerCharacter() const
{
    return Cast<ACharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
}

void ASearchEscapeGameMode::ApplyDamageToPlayer(float Amount, AActor* DamageInstigator)
{
    if (RunState != ESearchEscapeRunState::Playing)
    {
        return;
    }

    if (PlayerHealthComponent)
    {
        PlayerHealthComponent->TakeDamage(Amount, DamageInstigator);
    }
}

int32 ASearchEscapeGameMode::GetAliveEnemyCount() const
{
    int32 Count = 0;
    for (TActorIterator<ASearchEscapeEnemyCharacter> It(GetWorld()); It; ++It)
    {
        if (!It->IsDead())
        {
            Count++;
        }
    }
    return Count;
}

void ASearchEscapeGameMode::PrepareGameplayActorsForNewRun()
{
    BindGameplayActors();

    for (TActorIterator<ASearchEscapeChest> It(GetWorld()); It; ++It)
    {
        It->ResetChest();
    }

    for (TActorIterator<ASearchEscapeDoor> It(GetWorld()); It; ++It)
    {
        It->CancelAutoActivation();
        It->DeactivateDoor();
    }

    // Reset all enemies
    for (TActorIterator<ASearchEscapeEnemyCharacter> It(GetWorld()); It; ++It)
    {
        It->ResetEnemyForRun();
    }
}

void ASearchEscapeGameMode::BindGameplayActors()
{
    if (!GetWorld())
    {
        return;
    }

    for (TActorIterator<ASearchEscapeChest> It(GetWorld()); It; ++It)
    {
        ASearchEscapeChest* Chest = *It;
        Chest->OnChestOpened.RemoveDynamic(this, &ASearchEscapeGameMode::HandleChestOpened);
        Chest->OnChestOpened.AddDynamic(this, &ASearchEscapeGameMode::HandleChestOpened);
    }

    for (TActorIterator<ASearchEscapeDoor> It(GetWorld()); It; ++It)
    {
        ASearchEscapeDoor* Door = *It;
        Door->OnEscapeSucceeded.RemoveDynamic(this, &ASearchEscapeGameMode::HandleEscapeSucceeded);
        Door->OnEscapeSucceeded.AddDynamic(this, &ASearchEscapeGameMode::HandleEscapeSucceeded);
    }
}

void ASearchEscapeGameMode::BindEnemyDeathEvents()
{
    for (TActorIterator<ASearchEscapeEnemyCharacter> It(GetWorld()); It; ++It)
    {
        ASearchEscapeEnemyCharacter* Enemy = *It;
        if (Enemy->HealthComponent)
        {
            Enemy->HealthComponent->OnDeath.RemoveDynamic(this, &ASearchEscapeGameMode::HandleEnemyKilled);
            Enemy->HealthComponent->OnDeath.AddDynamic(this, &ASearchEscapeGameMode::HandleEnemyKilled);
        }
    }
}

void ASearchEscapeGameMode::ActivateEscapeDoors()
{
    bEscapeDoorsActivated = true;

    for (TActorIterator<ASearchEscapeDoor> It(GetWorld()); It; ++It)
    {
        It->ActivateDoor();
    }

    if (HUDWidget)
    {
        UKismetSystemLibrary::PrintString(this, TEXT("逃生门已激活！"), true, true, FLinearColor::Green, 3.0f);
    }
}

void ASearchEscapeGameMode::UpdateHUDState()
{
    if (!HUDWidget)
    {
        return;
    }

    HUDWidget->SetGold(CurrentGold);
    HUDWidget->SetTimeRemaining(TimeRemaining);
    HUDWidget->SetKillCount(KillCount);
    HUDWidget->SetCombatScore(CombatScore);
    HUDWidget->SetPlayerHealth(PlayerCurrentHealth, PlayerMaxHealth);

    if (RunState == ESearchEscapeRunState::WaitingToStart)
    {
        HUDWidget->ShowStartScreen();
    }
    else if (RunState == ESearchEscapeRunState::Playing)
    {
        HUDWidget->ShowPlayingHUD();
    }
}

void ASearchEscapeGameMode::HandleChestOpened(ASearchEscapeChest* Chest, AActor* Player, int32 GoldAmount)
{
    AddGold(GoldAmount);
}

void ASearchEscapeGameMode::HandleEscapeSucceeded(ASearchEscapeDoor* Door, AActor* Player)
{
    // Check if all enemies must be dead before escaping
    if (bRequireAllEnemiesDeadToEscape)
    {
        const int32 AliveEnemies = GetAliveEnemyCount();
        if (AliveEnemies > 0)
        {
            UKismetSystemLibrary::PrintString(this,
                FString::Printf(TEXT("还有 %d 个敌人存活，消灭所有敌人后方可撤离"), AliveEnemies),
                true, true, FLinearColor::Red, 3.0f);
            return;
        }
    }

    EndSearchEscapeGame(true, FText::FromString(TEXT("你已触碰逃生门，成功带出金币")));
}

void ASearchEscapeGameMode::HandlePlayerHealthDepleted(AActor* InOwner, AActor* Killer)
{
    EndSearchEscapeGame(false, FText::FromString(TEXT("生命值耗尽，未能撤离")));
}

void ASearchEscapeGameMode::HandlePlayerHealthChanged(AActor* InOwner, float NewHealth, float Delta)
{
    PlayerCurrentHealth = FMath::Clamp(NewHealth, 0.0f, PlayerMaxHealth);
    OnPlayerHealthChanged.Broadcast(PlayerCurrentHealth, PlayerMaxHealth);

    if (HUDWidget)
    {
        HUDWidget->SetPlayerHealth(PlayerCurrentHealth, PlayerMaxHealth);
    }
}

void ASearchEscapeGameMode::HandleEnemyKilled(AActor* Enemy, AActor* Killer)
{
    ASearchEscapeEnemyCharacter* EnemyChar = Cast<ASearchEscapeEnemyCharacter>(Enemy);
    if (!EnemyChar)
    {
        return;
    }

    // Add gold reward
    AddGold(EnemyChar->KillGoldReward);

    // Update combat score
    CombatScore += EnemyChar->KillScoreReward;
    OnCombatScoreChanged.Broadcast(CombatScore);
    if (HUDWidget)
    {
        HUDWidget->SetCombatScore(CombatScore);
    }

    // Update kill count
    KillCount++;
    OnKillCountChanged.Broadcast(KillCount);
    if (HUDWidget)
    {
        HUDWidget->SetKillCount(KillCount);
    }

    UKismetSystemLibrary::PrintString(this,
        FString::Printf(TEXT("击杀敌人：+%d 金币，战斗力 +%d"), EnemyChar->KillGoldReward, EnemyChar->KillScoreReward),
        true, false, FLinearColor(1.0f, 0.65f, 0.0f, 1.0f), 2.0f);
}
