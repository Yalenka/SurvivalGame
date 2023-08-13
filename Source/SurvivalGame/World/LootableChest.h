//Extended verison 2023 By [Yalenka] - SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LootableChest.generated.h"

UCLASS(ClassGroup = (Items), Blueprintable, Abstract)
class SURVIVALGAME_API ALootableChest : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALootableChest();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class UStaticMeshComponent* LootContainerMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class UInteractionComponent* LootInteraction;

	/**The items in the lootable actor are held in here*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class UInventoryComponent* Inventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class UDataTable* LootTable;

	//The number of times to roll the loot table. Random number between min and max will be used. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	FIntPoint LootRolls;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void OnInteract(class ASurvivalCharacter* Character);

};
