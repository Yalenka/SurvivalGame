#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ST_CameraHeight.generated.h"

USTRUCT(BlueprintType)
struct FST_CameraHeight : public FTableRowBase
{
	GENERATED_BODY()
	
	//Used to set new height for the character depends on the character new state/pose
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Height;
};