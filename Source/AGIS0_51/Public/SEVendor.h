#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SEVendor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class USearchEscapePlayerComponent;
class UUserWidget;

USTRUCT(BlueprintType)
struct FSEVendorItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vendor")
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vendor")
	int32 Price = 10;

	/** AGIS item row ID or asset path to give the player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vendor")
	FString ItemID;
};

UCLASS(Blueprintable)
class AGIS0_51_API ASEVendor : public AActor
{
	GENERATED_BODY()

public:
	ASEVendor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vendor|Components")
	TObjectPtr<UBoxComponent> InteractionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vendor|Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor|Shop")
	TArray<FSEVendorItem> ShopItems;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vendor|Shop")
	float InteractionRange = 300.0f;

	UFUNCTION(BlueprintCallable, Category = "Vendor")
	bool BuyItem(int32 ItemIndex, AActor* Buyer);

	UFUNCTION(BlueprintCallable, Category = "Vendor")
	bool CanPlayerAfford(AActor* Buyer, int32 ItemIndex) const;

	UFUNCTION(BlueprintPure, Category = "Vendor")
	FSEVendorItem GetItem(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = "Vendor")
	int32 GetItemCount() const { return ShopItems.Num(); }

	UFUNCTION(BlueprintImplementableEvent, Category = "Vendor|Events")
	void OnItemPurchased(int32 ItemIndex, AActor* Buyer);

	UFUNCTION(BlueprintImplementableEvent, Category = "Vendor|Events")
	void OnPurchaseFailed(const FString& Reason, AActor* Buyer);

	/** Called when player presses E while looking at this vendor */
	UFUNCTION(BlueprintCallable, Category = "Vendor")
	void Interact(AActor* Player);

	UFUNCTION(BlueprintImplementableEvent, Category = "Vendor|Events")
	void OnShopOpened(AActor* Player);

	UFUNCTION(BlueprintImplementableEvent, Category = "Vendor|Events")
	void OnPlayerEnterRange(AActor* Player);

	UFUNCTION(BlueprintImplementableEvent, Category = "Vendor|Events")
	void OnPlayerLeaveRange(AActor* Player);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	TSet<AActor*> PlayersInRange;
};
