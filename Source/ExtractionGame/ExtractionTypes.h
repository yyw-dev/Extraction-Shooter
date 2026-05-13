#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ExtractionTypes.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Pistol      UMETA(DisplayName = "Pistol"),
	Rifle       UMETA(DisplayName = "Rifle"),
	Shotgun     UMETA(DisplayName = "Shotgun")
};

UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	Weapon      UMETA(DisplayName = "Weapon"),
	Ammo        UMETA(DisplayName = "Ammo"),
	Health      UMETA(DisplayName = "Health"),
	Valuables   UMETA(DisplayName = "Valuables"),
	Misc        UMETA(DisplayName = "Misc")
};

UENUM(BlueprintType)
enum class EContainerType : uint8
{
	Crate       UMETA(DisplayName = "Crate"),
	DuffleBag   UMETA(DisplayName = "DuffleBag"),
	WeaponCase  UMETA(DisplayName = "WeaponCase")
};

USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemCategory Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;
};

USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FireRate = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MagazineSize = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 10000.0f;
};

USTRUCT(BlueprintType)
struct FContainerLootEntry : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemCategory Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnChance = 1.0f;
};
