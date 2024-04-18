//Extended verison 2023 By [Yalenka] - SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020

#pragma once

#include "CoreMinimal.h"
#include "EquippableItem.h"
#include "ThrowableItem.generated.h"

/**
 * Throwable item is the base class for items that can be thrown, such as grenades.
 */
UCLASS(Blueprintable)
class SURVIVALGAME_API UThrowableItem : public UEquippableItem
{
	GENERATED_BODY()
	
public:

	UThrowableItem();

	//The montage to play when we toss a throwable
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	class UAnimMontage* ThrowableTossAnimation;

	//The actor to spawn in when we throw the item. (ie grenade actor, molotov actor, etc)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<class AThrowableWeapon> ThrowableClass;

};
