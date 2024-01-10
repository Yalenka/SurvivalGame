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
