//Extended verison 2023 By [Yalenka] - SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020


#include "WeaponItem.h"
#include "../Player/SurvivalPlayerController.h"
#include "../Player/SurvivalCharacter.h"

UWeaponItem::UWeaponItem()
{

}

bool UWeaponItem::Equip(class ASurvivalCharacter* Character)
{
	bool bEquipSuccessful = Super::Equip(Character);

	if (bEquipSuccessful && Character)
	{
		Character->EquipWeapon(this);
	}

	return bEquipSuccessful;
}

bool UWeaponItem::UnEquip(class ASurvivalCharacter* Character)
{
	bool bUnEquipSuccessful = Super::UnEquip(Character);

	if (bUnEquipSuccessful && Character)
	{
		Character->UnEquipWeapon();
		Character->DropItem(this, this->GetQuantity());
	}

	return bUnEquipSuccessful;
}
