//Extended verison 2023 By [Yalenka] - SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "SurvivalDamageTypes.generated.h"

UCLASS()
class USurvivalDamageBase : public UDamageType
{
	GENERATED_BODY()
};

UCLASS()
class UWeaponDamage : public USurvivalDamageBase
{
	GENERATED_BODY()
};

UCLASS()
class UMeleeDamage : public USurvivalDamageBase
{
	GENERATED_BODY()
};

UCLASS()
class UExplosiveDamage : public USurvivalDamageBase
{
	GENERATED_BODY()
};

UCLASS()
class UVehicleDamage : public USurvivalDamageBase
{
	GENERATED_BODY()
};