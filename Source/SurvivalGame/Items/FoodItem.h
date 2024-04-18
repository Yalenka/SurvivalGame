//Extended verison 2023 By [Yalenka] - SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "FoodItem.generated.h"

/**
 * Food item is the base class for an item that gives health when consumed...
 */
UCLASS()
class SURVIVALGAME_API UFoodItem : public UItem
{
	GENERATED_BODY()
	
public:

	UFoodItem();

	/**The amount for the food to heal*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Healing")
		float HealAmount;

	virtual void Use(class ASurvivalCharacter* Character) override;
};
