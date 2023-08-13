#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ST_WalkSpeed.generated.h"

USTRUCT(BlueprintType)
struct FST_WalkSpeed : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Forward;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Others;
};