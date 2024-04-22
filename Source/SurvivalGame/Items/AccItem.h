// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EquippableItem.h"
#include "AccItem.generated.h"

UCLASS(Blueprintable)
class SURVIVALGAME_API UAccItem : public UEquippableItem
{
	GENERATED_BODY()

public:

	//Acc type, mag, muzzle, sight etc
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Data")
	EWeaponAccType AccType;

	//ID's of weapons which will support this acc item
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Data")
	TArray<FName> WeaponIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	class UStaticMesh* AccMesh;

	//Defining the socket name for the item to be attached
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	FName AttachmentSocket;

	//Muzzle Related
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Data")
	bool IsSuppressor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Data")
	bool bCanFlash;

	//Mag Related
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	float ClipRate;
};
