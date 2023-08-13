//Extended verison 2023 By [Yalenka] - SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryItemWidget.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API UInventoryItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, Category = "Inventory Item Widget", meta = (ExposeOnSpawn = true))
		class UItem* Item;

	/**Used to access the pickup actor in the UI, which is belongs from the UItem, so we can destroy the pickup actor in level while on dragged if we pick it up using UI */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory Item Widget", meta = (ExposeOnSpawn = true))
		class APickup* PickupActor;
};
