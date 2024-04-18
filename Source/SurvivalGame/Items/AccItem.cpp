// Fill out your copyright notice in the Description page of Project Settings.


#include "AccItem.h"
#include "../Player/SurvivalCharacter.h"

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
