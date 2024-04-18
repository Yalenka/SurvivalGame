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
		//Dont auto equip item if we are looting from another player
		if (Character && !Character->IsLooting())
		{
			if (!Character->GetEquippedItems().Contains(Slot) && !(ItemType == EItemType::E_Weapon) /* && !(ItemType == EItemType::E_Accessories)*/)
			{
				if (ItemType == EItemType::E_Grenade)
				{
					if (!Character->GetEquippedItems().Contains(EEquippableSlot::EIS_Throwable))
					{
						if (!Character->GetHoldWeapon())
						{
							SetEquipped(true);
						}
					}
				}
				else
				{
					if (!(ItemType == EItemType::E_Accessories))
					{
						//Directly equip any items like: backpack, helmet, shirt
						SetEquipped(true);
					}
				}
			}
			else
			{
				if (ItemType == EItemType::E_Weapon)
				{
					SetEquipped(true);
				}
				/*UEquippableItem** Primary = Character->GetEquippedItems().Find(EEquippableSlot::EIS_PrimaryWeapon);
				UEquippableItem** Secondary = Character->GetEquippedItems().Find(EEquippableSlot::EIS_SecondaryWeapon);
				switch (Slot)
				{
				case EEquippableSlot::EIS_PrimaryWeapon:
				{
					if (Primary && Secondary)
					{
						if (Character->GetHoldWeapon())
						{
							SetEquipped(false);
							if (!Character->GetEquippedItems().Contains(Slot))
							{
								SetEquipped(true);
							}
						}
					}
					else if (Primary)
					{
						(*Primary)->SetEquipped(false);
						if (!Character->GetEquippedItems().Contains(Slot))
						{
							SetEquipped(true);
						}
					}
					else
					{
						SetEquipped(true);
					}
				}
				break;
				case EEquippableSlot::EIS_SecondaryWeapon:
				{
					if (Primary && Secondary)
					{
						if (Character->GetHoldWeapon())
						{
							Character->UnEquipWeapon();
							if (!Character->GetEquippedItems().Contains(Slot))
							{
								SetEquipped(true);
							}
						}
					}
					else if (Secondary)
					{
						(*Secondary)->SetEquipped(false);
						if (!Character->GetEquippedItems().Contains(Slot))
						{
							SetEquipped(true);
						}
					}
					else
					{
						SetEquipped(true);
					}
				}
				break;
				default:
					break;
				}*/
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