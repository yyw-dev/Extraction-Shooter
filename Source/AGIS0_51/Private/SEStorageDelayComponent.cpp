#include "SEStorageDelayComponent.h"

USEStorageDelayComponent::USEStorageDelayComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USEStorageDelayComponent::RequestOpen(AActor* Interactor)
{
	if (bIsOpening || !Interactor) return;

	bIsOpening = true;
	OpenProgress = 0.0f;
	ElapsedTime = 0.0f;
	PendingInteractor = Interactor;

	// Enable tick for progress updates
	PrimaryComponentTick.SetTickFunctionEnable(true);
}

void USEStorageDelayComponent::CancelOpen()
{
	if (!bIsOpening) return;

	bIsOpening = false;
	OpenProgress = 0.0f;
	PendingInteractor.Reset();
	PrimaryComponentTick.SetTickFunctionEnable(false);
}

void USEStorageDelayComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsOpening) return;

	ElapsedTime += DeltaTime;
	OpenProgress = FMath::Clamp(ElapsedTime / OpenDelay, 0.0f, 1.0f);
	OnProgressUpdated.Broadcast();

	if (ElapsedTime >= OpenDelay)
	{
		CompleteOpen();
	}
}

void USEStorageDelayComponent::CompleteOpen()
{
	bIsOpening = false;
	OpenProgress = 1.0f;
	PrimaryComponentTick.SetTickFunctionEnable(false);

	if (AActor* Interactor = PendingInteractor.Get())
	{
		OnOpenComplete.Broadcast(Interactor);
	}
	PendingInteractor.Reset();
}
