//Extended verison 2023 By [Yalenka] - SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020


#include "EquippableItem.h"
#include "Net/UnrealNetwork.h"

#include "../Player/SurvivalCharacter.h"
#include "../Components/InventoryComponent.h"

#define LOCTEXT_NAMESPACE "EquippableItem"

UEquippableItem::UEquippableItem()
{
	bStackable = false;
	bEquipped = false;
	UseActionText = LOCTEXT("EquipText", "Equip");
}

void UEquippableItem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEquippableItem, bEquipped);
}

void UEquippableItem::Use(class ASurvivalCharacter* Character)
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

bool UEquippableItem::Equip(class ASurvivalCharacter* Character)
{
	if (Character)
	{
		return Character->EquipItem(this);
	}
	return false;
}

bool UEquippableItem::UnEquip(class ASurvivalCharacter* Character)
{
	if (Character)
	{
		return Character->UnEquipItem(this);
	}
	return false;
}

bool UEquippableItem::ShouldShowInInventory() const
{
	return !bEquipped;
}

void UEquippableItem::AddedToInventory(class UInventoryComponent* Inventory)
{
	//If the player looted an item don't equip it
	if (ASurvivalCharacter* Character = Cast<ASurvivalCharacter>(Inventory->GetOwner()))
	{
		if (Character && !Character->IsLooting())
		{
			/**If we take an equippable, and don't have an item equipped at its slot, then auto equip it*/
			if (!Character->GetEquippedItems().Contains(Slot))
			{
				SetEquipped(true);
			}
		}
	}
}

void UEquippableItem::SetEquipped(bool bNewEquipped)
{
	bEquipped = bNewEquipped;
	EquipStatusChanged();
	MarkDirtyForReplication();
}

void UEquippableItem::EquipStatusChanged()
{
	if (ASurvivalCharacter* Character = Cast<ASurvivalCharacter>(GetOuter()))
	{
		UseActionText = bEquipped ? LOCTEXT("UnequipText", "Unequip") : LOCTEXT("EquipText", "Equip");

		if (bEquipped)
		{
			Equip(Character);
		}
		else
		{
			UnEquip(Character);
		}
	}

	//Tell ui to update
	OnItemModified.Broadcast();
}

#undef LOCTEXT_NAMESPACE 