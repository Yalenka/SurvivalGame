// SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SurvivalStatics.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API USurvivalStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

		UFUNCTION(BlueprintPure, Category = "Utilities")
		static UClass* GetObjectParentClass(UObject* Object);

};
