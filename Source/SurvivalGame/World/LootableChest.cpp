//Extended verison 2023 By [Yalenka] - SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020


#include "LootableChest.h"
#include "Components/InteractionComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "World/ItemSpawn.h"
#include "Items/Item.h"
#include "Player/SurvivalCharacter.h"

#define LOCTEXT_NAMESPACE "LootableChest"


// Sets default values
ALootableChest::ALootableChest()
{
	LootContainerMesh = CreateDefaultSubobject<UStaticMeshComponent>("LootContainerMesh");
	SetRootComponent(LootContainerMesh);

	LootInteraction = CreateDefaultSubobject<UInteractionComponent>("LootInteraction");
	LootInteraction->InteractableActionText = LOCTEXT("LootActorText", "Loot");
	LootInteraction->InteractableNameText = LOCTEXT("LootActorName", "Chest");
	LootInteraction->SetupAttachment(GetRootComponent());

	Inventory = CreateDefaultSubobject<UInventoryComponent>("Inventory");
	Inventory->SetCapacity(20);
	Inventory->SetWeightCapacity(80.f);

	LootRolls = FIntPoint(2, 8);

	SetReplicates(true);
}

// Called when the game starts or when spawned
void ALootableChest::BeginPlay()
{
	Super::BeginPlay();
	
	LootInteraction->OnInteract.AddDynamic(this, &ALootableChest::OnInteract);

	if (HasAuthority() && LootTable)
	{
		TArray<FLootTableRow*> SpawnItems;
		LootTable->GetAllRows("", SpawnItems);

		int32 Rolls = FMath::RandRange(LootRolls.GetMin(), LootRolls.GetMax());

		for (int32 i = 0; i < Rolls; ++i)
		{
			const FLootTableRow* LootRow = SpawnItems[FMath::RandRange(0, SpawnItems.Num() - 1)];

			ensure(LootRow);

			float ProbabilityRoll = FMath::FRandRange(0.f, 1.f);

			while (ProbabilityRoll > LootRow->Probability)
			{
				LootRow = SpawnItems[FMath::RandRange(0, SpawnItems.Num() - 1)];
				ProbabilityRoll = FMath::FRandRange(0.f, 1.f);
			}

			if (LootRow && LootRow->Items.Num())
			{
				for (auto& ItemClass : LootRow->Items)
				{
					if (ItemClass)
					{
						const int32 Quantity = Cast<UItem>(ItemClass->GetDefaultObject())->GetQuantity();
						Inventory->TryAddItemFromClass(ItemClass, Quantity);
					}
				}
			}
		}
	}
}

void ALootableChest::OnInteract(class ASurvivalCharacter* Character)
{
	if (Character)
	{
		Character->SetLootSource(Inventory);
	}
}

#undef LOCTEXT_NAMESPACE