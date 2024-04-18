// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EquippableItem.h"
#include "AccItem.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class SURVIVALGAME_API UAccItem : public UEquippableItem
{
	GENERATED_BODY()

public:

	//Acc type, mag, muzzle, sight etc
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item Extras")
	EWeaponAccType AccType;

	//ID's of weapons which will support this acc item
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components|Weapon Accessories")
	TArray<FName> WeaponIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Weapon Accessories")
	class UStaticMesh* AccMesh;

	//Defining the socket name for the item to be attached
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Weapon Accessories")
	FName AttachmentSocket;

	//Muzzle Related
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components|Weapon Accessories")
	bool IsSuppressor;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components|Weapon Accessories")
	bool bCanFlash;

	//Mag Related
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components|Weapon Accessories")
	float ClipRate;

	virtual bool Equip(class ASurvivalCharacter* Character) override;
	virtual bool UnEquip(class ASurvivalCharacter* Character) override;
};
