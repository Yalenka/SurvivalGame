#pragma once

#include "CoreMinimal.h"
#include "SurvivalEnums.generated.h"

/** Weapon Positions enum on character...*/
UENUM(BlueprintType)
enum class EWeaponPosition : uint8
{
	E_Left UMETA(DisplayName = "LeftPosition"),
	E_Right UMETA(DisplayName = "RightPosition"),
	E_Pan UMETA(DisplayName = "PanPosition"),
	E_Grenade UMETA(DisplayName = "GrenadePosition"),
	E_MAX UMETA(DisplayName = "EmptyPosition")

};
/** All Items Type Enum*/
UENUM(BlueprintType)
enum class EItemType : uint8 /*Defining the Equipment item type*/
{
	E_None UMETA(DisplayName = "None"),
	E_Weapon UMETA(DisplayName = "Weapon"),
	E_Accessories UMETA(DisplayName = "Accessories"),
	E_Ammo UMETA(DisplayName = "Ammo"),
	E_Health UMETA(DisplayName = "Health"),
	E_Boost UMETA(DisplayName = "Boost"),
	E_Helmet UMETA(DisplayName = "Helmet"),
	E_Vest UMETA(DisplayName = "Vest"),
	E_BackPack UMETA(DisplayName = "BackPack"),
	E_Fashion UMETA(DisplayName = "Fashion"),
	E_Pan UMETA(DisplayName = "Pan"),
	E_Grenade UMETA(DisplayName = "Grenade")
};

UENUM(BlueprintType)
enum class EMontageType : uint8
{
	EIS_Equip UMETA(DisplayName = "Equip"),
	EIS_UnEquip UMETA(DisplayName = "UnEquip"),
	EIS_Reload UMETA(DisplayName = "Reload"),
	EIS_ReloadBullet UMETA(DisplayName = "ReloadBullet"),
	EIS_Fire UMETA(DisplayName = "Fire"),
	EIS_Use UMETA(DisplayName = "Use"),
	EIS_Pickup UMETA(DisplayName = "Pickup"),
	EIS_ThrowGrenade UMETA(DisplayName = "ThrowGrenade"),
	EIS_ThrowPetrolBomb UMETA(DisplayName = "ThrowPetrolBomb")
};

///** Weapon Acc Item Type Enum*/
UENUM(BlueprintType)
enum class EWeaponAccType : uint8
{
	E_None UMETA(DisplayName = "None"),
	E_Sight UMETA(DisplayName = "Sight"),
	E_Mag UMETA(DisplayName = "Mag"),
	E_Muzzle UMETA(DisplayName = "Muzzle")
};

//All the slots that gear can be equipped to.
UENUM(BlueprintType)
enum class EEquippableSlot : uint8
{
	EIS_Head UMETA(DisplayName = "Head"),
	EIS_Helmet UMETA(DisplayName = "Helmet"),
	EIS_Chest UMETA(DisplayName = "Chest"),
	EIS_Vest UMETA(DisplayName = "Vest"),
	EIS_Legs UMETA(DisplayName = "Legs"),
	EIS_Feet UMETA(DisplayName = "Feet"),
	EIS_Hands UMETA(DisplayName = "Hands"),
	EIS_Backpack UMETA(DisplayName = "Backpack"),
	EIS_PrimaryWeapon UMETA(DisplayName = "Primary Weapon"),
	EIS_SecondaryWeapon UMETA(DisplayName = "Secondary Weapon"),
	EIS_Throwable UMETA(DisplayName = "Throwable Item"),
	EIS_Mag UMETA(DisplayName = "Mag"),
	EIS_Muzzle UMETA(DisplayName = "Muzzle"),
	EIS_None UMETA(DisplayName = "None")
};

UENUM(BlueprintType)
enum class EInventoryLocation : uint8
{
	E_None UMETA(DisplayName = "None"),
	E_BagPack UMETA(DisplayName = "BagPack"),
	E_Ground UMETA(DisplayName = "Ground"),
	E_Weapon UMETA(DisplayName = "Weapon"),
	E_WeaponAcc UMETA(DisplayName = "WeaponAcc")
};