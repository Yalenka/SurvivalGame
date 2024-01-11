// Fill out your copyright notice in the Description page of Project Settings.


#include "AccItem.h"
#include "../Player/SurvivalCharacter.h"

void UAccItem::Use(class ASurvivalCharacter* Character)
{
	if (Character && Character->HasAuthority())
	{
		if (Character->GetEquippedItems().Contains(Slot) && !bEquipped)
		{
			UEquippableItem* AlreadyEquippedItem = *Character->GetEquippedItems().Find(Slot);

			AlreadyEquippedItem->SetEquipped(false);
		}

		SetEquipped(!IsEquipped());
	}
}

bool UAccItem::Equip(class ASurvivalCharacter* Character)
{
	bool bEquipSuccessful = Super::Equip(Character);

	if (bEquipSuccessful && Character)
	{
		Character->EquipAcc(this);
	}

	return bEquipSuccessful;
}

bool UAccItem::UnEquip(class ASurvivalCharacter* Character)
{
	bool bUnEquipSuccessful = Super::UnEquip(Character);

	if (bUnEquipSuccessful && Character)
	{
		Character->UnEquipAcc(Slot, this);
	}

	return bUnEquipSuccessful;
}
