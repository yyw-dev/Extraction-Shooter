#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SearchEscapeDoor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSearchEscapeDoorUsedSignature, ASearchEscapeDoor*, Door, AActor*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSearchEscapeDoorStateChangedSignature, ASearchEscapeDoor*, Door, bool, bIsActive);

UCLASS(Blueprintable)
class AGIS0_51_API ASearchEscapeDoor : public AActor
{
    GENERATED_BODY()

public:
    ASearchEscapeDoor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Escape Door|Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Escape Door|Components")
    TObjectPtr<UStaticMeshComponent> LeftPostMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Escape Door|Components")
    TObjectPtr<UStaticMeshComponent> RightPostMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Escape Door|Components")
    TObjectPtr<UStaticMeshComponent> TopBeamMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Escape Door|Components")
    TObjectPtr<UBoxComponent> EscapeTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escape Door|Settings")
    FName DoorId = TEXT("A");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escape Door|Settings", meta = (ClampMin = "0.0"))
    float ActivationDelay = 240.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escape Door|Settings")
    bool bAutoActivateAfterDelay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escape Door|Settings")
    bool bStartActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escape Door|Settings")
    bool bConsumeOnUse = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Escape Door|Settings")
    bool bPrintDebugMessages = true;

    UPROPERTY(BlueprintReadOnly, Category = "Escape Door|State")
    bool bIsActive = false;

    UPROPERTY(BlueprintAssignable, Category = "Escape Door|Events")
    FSearchEscapeDoorUsedSignature OnEscapeSucceeded;

    UPROPERTY(BlueprintAssignable, Category = "Escape Door|Events")
    FSearchEscapeDoorStateChangedSignature OnEscapeDoorStateChanged;

    UFUNCTION(BlueprintCallable, Category = "Escape Door")
    void ActivateDoor();

    UFUNCTION(BlueprintCallable, Category = "Escape Door")
    void DeactivateDoor();

    UFUNCTION(BlueprintCallable, Category = "Escape Door")
    void SetDoorActive(bool bNewActive);

    UFUNCTION(BlueprintCallable, Category = "Escape Door")
    void CancelAutoActivation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Escape Door|Events")
    void ReceiveEscapeSucceeded(AActor* Player);

protected:
    virtual void BeginPlay() override;

private:
    FTimerHandle ActivationTimerHandle;
    bool bConsumed = false;

    UFUNCTION()
    void OnEscapeTriggerBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
