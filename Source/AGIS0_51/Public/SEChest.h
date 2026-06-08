#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SEChest.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class ASEPlayerCharacter;

UCLASS(Blueprintable)
class AGIS0_51_API ASEChest : public AActor
{
	GENERATED_BODY()

public:
	ASEChest();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SearchEscape|Chest")
	TObjectPtr<UBoxComponent> InteractionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SearchEscape|Chest")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SearchEscape|Chest")
	float SE_OpenDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SearchEscape|Chest")
	float SE_InteractionRadius = 210.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SearchEscape|Chest")
	int32 SE_MinGold = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SearchEscape|Chest")
	int32 SE_MaxGold = 50;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SearchEscape|Chest")
	float SE_OpenProgress = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SearchEscape|Chest")
	bool SE_IsOpened = false;

protected:
	virtual void BeginPlay() override;

private:
	ASEPlayerCharacter* GetNearbyPlayer() const;
	void OpenChest(ASEPlayerCharacter* Player);
};
