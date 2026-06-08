#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SearchEscapeChest.generated.h"

class USphereComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSearchEscapeChestOpenedSignature, ASearchEscapeChest*, Chest, AActor*, Player, int32, GoldAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSearchEscapeChestProgressSignature, ASearchEscapeChest*, Chest, float, Progress01);

UCLASS(Blueprintable)
class AGIS0_51_API ASearchEscapeChest : public AActor
{
    GENERATED_BODY()

public:
    ASearchEscapeChest();

    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chest|Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chest|Components")
    TObjectPtr<UStaticMeshComponent> BaseMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chest|Components")
    TObjectPtr<USceneComponent> LidPivot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chest|Components")
    TObjectPtr<UStaticMeshComponent> LidMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chest|Components")
    TObjectPtr<USphereComponent> InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Settings", meta = (ClampMin = "0.1"))
    float OpenDuration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Settings", meta = (ClampMin = "0"))
    int32 MinGold = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Settings", meta = (ClampMin = "0"))
    int32 MaxGold = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest|Settings")
    bool bPrintDebugMessages = true;

    UPROPERTY(BlueprintReadOnly, Category = "Chest|State")
    bool bOpened = false;

    UPROPERTY(BlueprintReadOnly, Category = "Chest|State")
    float OpenProgress = 0.0f;

    UPROPERTY(BlueprintAssignable, Category = "Chest|Events")
    FSearchEscapeChestOpenedSignature OnChestOpened;

    UPROPERTY(BlueprintAssignable, Category = "Chest|Events")
    FSearchEscapeChestProgressSignature OnChestProgressChanged;

    UFUNCTION(BlueprintCallable, Category = "Chest")
    void ResetChest();

    UFUNCTION(BlueprintPure, Category = "Chest")
    bool IsPlayerOpening() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Chest|Events")
    void ReceiveChestOpened(AActor* Player, int32 GoldAmount);

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TObjectPtr<AActor> OpeningPlayer;

    UFUNCTION()
    void OnInteractionBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void SetOpenVisual(float Progress01);
    void CompleteOpen();
};
