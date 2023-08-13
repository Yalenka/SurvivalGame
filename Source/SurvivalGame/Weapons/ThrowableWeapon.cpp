//Extended verison 2023 By [Yalenka] - SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020


#include "ThrowableWeapon.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AThrowableWeapon::AThrowableWeapon()
{
	ThrowableMesh = CreateDefaultSubobject<UStaticMeshComponent>("ThrowableMesh");
	SetRootComponent(ThrowableMesh);

	ThrowableMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ThrowableMovement");
	ThrowableMovement->InitialSpeed = 1000.f;

	SetReplicates(true);
	SetReplicateMovement(true);
}

