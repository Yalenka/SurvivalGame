//Extended verison 2023 By [Yalenka] - SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/TargetPoint.h"
#include "ItemSpawn.generated.h"

USTRUCT(BlueprintType)
struct FLootTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	//The item(s) to spawn
	UPROPERTY(EditDefaultsOnly, Category = "Loot")
		TArray<TSubclassOf<class UItem>> Items;

	//The percentage chance of spawning this item if we hit it on the roll
	UPROPERTY(EditDefaultsOnly, Category = "Loot", meta = (ClampMin = 0.001, ClampMax = 1.0))
		float Probability = 1.f;

};

/**
 * 
 */
UCLASS(ClassGroup = (Items), Blueprintable, Abstract)
class SURVIVALGAME_API AItemSpawn : public ATargetPoint
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AItemSpawn();

	UPROPERTY(EditAnywhere, Category = "Loot")
	class UDataTable* LootTable;

	/**Because pickups use a Blueprint base, we use a UPROPERTY to select it*/
	UPROPERTY(EditDefaultsOnly, Category = "Loot")
	TSubclassOf<class APickup> PickupClass;

	/**Range used for generating respawn time*/
	UPROPERTY(EditDefaultsOnly, Category = "Loot")
	FIntPoint RespawnRange;

protected:

	FTimerHandle TimerHandle_RespawnItem;

	UPROPERTY()
	TArray<AActor*> SpawnedPickups;

	virtual void BeginPlay() override;

	UFUNCTION()
		void SpawnItem();

	//This is bound to the item being destroyed, so we can queue up another item to be spawned in
	UFUNCTION()
		void OnItemTaken(AActor* DestroyedActor);
};
