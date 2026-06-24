#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SEStorageDelayComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AGIS0_51_API USEStorageDelayComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USEStorageDelayComponent();

	/** How long to wait before opening (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="StorageDelay")
	float OpenDelay = 1.0f;

	/** Current progress 0..1, broadcast for UI progress bar */
	UPROPERTY(BlueprintReadOnly, Category="StorageDelay")
	float OpenProgress = 0.0f;

	/** Whether the box is currently being opened */
	UPROPERTY(BlueprintReadOnly, Category="StorageDelay")
	bool bIsOpening = false;

	/** Start the delayed open. Call this instead of directly opening. */
	UFUNCTION(BlueprintCallable, Category="StorageDelay")
	void RequestOpen(AActor* Interactor);

	/** Cancel an in-progress open */
	UFUNCTION(BlueprintCallable, Category="StorageDelay")
	void CancelOpen();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDelayedOpen, AActor*, Interactor);
	UPROPERTY(BlueprintAssignable, Category="StorageDelay|Events")
	FOnDelayedOpen OnOpenComplete;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProgressChanged);
	UPROPERTY(BlueprintAssignable, Category="StorageDelay|Events")
	FOnProgressChanged OnProgressUpdated;

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	FTimerHandle DelayTimer;
	TWeakObjectPtr<AActor> PendingInteractor;
	float ElapsedTime = 0.0f;

	void CompleteOpen();
	void UpdateProgress();
};
