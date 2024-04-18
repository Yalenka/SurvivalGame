//Extended verison 2023 By [Yalenka] - SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SurvivalGameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API ASurvivalGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:

	ASurvivalGameStateBase();

	UPROPERTY(EditDefaultsOnly, Category = "Game Rules")
	float RespawnTime;

};
