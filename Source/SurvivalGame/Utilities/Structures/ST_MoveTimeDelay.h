#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ST_MoveTimeDelay.generated.h"

/*
This struct is used to control the delay time while switching the state pose
Example:: Character should not move while in state from `prone to stand` or `stand to prone` ect....
Set False `EnableMove` when switching states according to the animation time Dilation.
If Animation time from prone to stand is 2 seconds, disable movement for 2 sec until the animation completes... after 2 sec re-enable it. "bEnableMove = false / true"
*/
USTRUCT(BlueprintType)
struct FST_MoveTimeDelay : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Sec;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FText Details;

};