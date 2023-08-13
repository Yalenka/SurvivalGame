//Extended verison 2023 By [Yalenka] - SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemTooltip.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API UItemTooltip : public UUserWidget
{
	GENERATED_BODY()

public:

	/**The item this tooltip should display*/
	UPROPERTY(BlueprintReadWrite, Category = "Tooltip", meta = (ExposeOnSpawn = true))
	class UItem* TooltipItem;
};
