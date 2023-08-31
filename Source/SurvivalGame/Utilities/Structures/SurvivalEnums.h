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