// SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NotificationWidget.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API UNotificationWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintImplementableEvent)
	void ShowNotification(const FText& NotificationText);

};
