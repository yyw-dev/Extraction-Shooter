#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SEEscapeDoor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class AGIS0_51_API ASEEscapeDoor : public AActor
{
	GENERATED_BODY()

public:
	ASEEscapeDoor();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "SearchEscape|Escape")
	void SE_SetActive(bool bNewActive);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SearchEscape|Escape")
	TObjectPtr<UBoxComponent> EscapeBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SearchEscape|Escape")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SearchEscape|Escape")
	float SE_InteractionRadius = 220.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SearchEscape|Escape")
	bool SE_IsActive = false;

protected:
	virtual void BeginPlay() override;
};
