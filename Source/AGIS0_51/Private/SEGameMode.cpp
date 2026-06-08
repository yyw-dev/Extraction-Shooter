#include "SEGameMode.h"

#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "SEChest.h"
#include "SEEscapeDoor.h"
#include "SEHUD.h"
#include "SEMonster.h"
#include "SEPlayerCharacter.h"

namespace SearchEscapeTags
{
	static const FName ChestMarker(TEXT("SE.ChestMarker"));
	static const FName MonsterSpawn(TEXT("SE.MonsterSpawn"));
	static const FName EscapePoint(TEXT("SE.EscapePoint"));
}

ASEGameMode::ASEGameMode()
{
	DefaultPawnClass = ASEPlayerCharacter::StaticClass();
	HUDClass = ASEHUD::StaticClass();
	SE_ResultTitle = FText::FromString(TEXT("Playing"));
	SE_ResultDetail = FText::GetEmpty();
}

void ASEGameMode::BeginPlay()
{
	Super::BeginPlay();
	SE_StartRound();
}

void ASEGameMode::SE_StartRound()
{
	SE_TimeRemaining = SE_RoundDuration;
	SE_TotalGold = 0;
	SE_GameEnded = false;
	SE_EscapeDoorsActive = false;
	SE_ResultTitle = FText::FromString(TEXT("Playing"));
	SE_ResultDetail = FText::GetEmpty();
	EscapeDoors.Reset();

	SpawnActorsFromMarkers();

	GetWorldTimerManager().ClearTimer(RoundTimerHandle);
	GetWorldTimerManager().SetTimer(RoundTimerHandle, this, &ASEGameMode::TickRoundTimer, 1.0f, true);
}

void ASEGameMode::TickRoundTimer()
{
	if (SE_GameEnded)
	{
		return;
	}

	SE_TimeRemaining = FMath::Max(0, SE_TimeRemaining - 1);

	const int32 Elapsed = SE_RoundDuration - SE_TimeRemaining;
	if (!SE_EscapeDoorsActive && Elapsed >= SE_EscapeDoorSpawnTime)
	{
		ActivateEscapeDoors();
	}

	if (SE_TimeRemaining <= 0)
	{
		SE_EndGameTimeOut();
	}
}

void ASEGameMode::SpawnActorsFromMarkers()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TArray<AActor*> ChestMarkers;
	UGameplayStatics::GetAllActorsWithTag(World, SearchEscapeTags::ChestMarker, ChestMarkers);
	for (AActor* Marker : ChestMarkers)
	{
		if (!Marker || Marker->IsA<ASEChest>())
		{
			continue;
		}

		World->SpawnActor<ASEChest>(ASEChest::StaticClass(), Marker->GetActorTransform());
		Marker->SetActorHiddenInGame(true);
		Marker->SetActorEnableCollision(false);
	}

	TArray<AActor*> MonsterMarkers;
	UGameplayStatics::GetAllActorsWithTag(World, SearchEscapeTags::MonsterSpawn, MonsterMarkers);
	for (AActor* Marker : MonsterMarkers)
	{
		if (!Marker || Marker->IsA<ASEMonster>())
		{
			continue;
		}

		World->SpawnActor<ASEMonster>(ASEMonster::StaticClass(), Marker->GetActorTransform());
		Marker->SetActorHiddenInGame(true);
		Marker->SetActorEnableCollision(false);
	}

	TArray<AActor*> EscapeMarkers;
	UGameplayStatics::GetAllActorsWithTag(World, SearchEscapeTags::EscapePoint, EscapeMarkers);
	for (AActor* Marker : EscapeMarkers)
	{
		if (!Marker)
		{
			continue;
		}

		ASEEscapeDoor* Door = Marker->IsA<ASEEscapeDoor>() ? Cast<ASEEscapeDoor>(Marker) : World->SpawnActor<ASEEscapeDoor>(ASEEscapeDoor::StaticClass(), Marker->GetActorTransform());
		if (Door)
		{
			Door->SE_SetActive(false);
			EscapeDoors.Add(Door);
		}

		if (!Marker->IsA<ASEEscapeDoor>())
		{
			Marker->SetActorHiddenInGame(true);
			Marker->SetActorEnableCollision(false);
		}
	}
}

void ASEGameMode::ActivateEscapeDoors()
{
	SE_EscapeDoorsActive = true;
	for (ASEEscapeDoor* Door : EscapeDoors)
	{
		if (Door)
		{
			Door->SE_SetActive(true);
		}
	}
}

void ASEGameMode::SE_EndGameEscape()
{
	EndGame(
		FText::FromString(TEXT("Escape Successful")),
		FText::FromString(TEXT("You reached an escape door and extracted safely.")));
}

void ASEGameMode::SE_EndGameDead()
{
	EndGame(
		FText::FromString(TEXT("You Died")),
		FText::FromString(TEXT("Health reached 0 before extraction.")));
}

void ASEGameMode::SE_EndGameTimeOut()
{
	EndGame(
		FText::FromString(TEXT("Time Out")),
		FText::FromString(TEXT("The 5-minute round ended before extraction.")));
}

void ASEGameMode::EndGame(const FText& Title, const FText& Detail)
{
	if (SE_GameEnded)
	{
		return;
	}

	SE_GameEnded = true;
	SE_ResultTitle = Title;
	SE_ResultDetail = Detail;
	GetWorldTimerManager().ClearTimer(RoundTimerHandle);

	if (ASEPlayerCharacter* Player = Cast<ASEPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
	{
		SE_TotalGold = Player->SE_Gold;
	}

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		PC->SetShowMouseCursor(true);
	}
}

FString ASEGameMode::GetTimerText() const
{
	const int32 Minutes = SE_TimeRemaining / 60;
	const int32 Seconds = SE_TimeRemaining % 60;
	return FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
}

float ASEGameMode::GetRoundProgress() const
{
	return SE_RoundDuration > 0 ? static_cast<float>(SE_RoundDuration - SE_TimeRemaining) / static_cast<float>(SE_RoundDuration) : 0.0f;
}
