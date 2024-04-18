//Extended verison 2023 By [Yalenka] - SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS(ClassGroup = (Items), Blueprintable, Abstract)
class SURVIVALGAME_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickup();

	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//always use VisibleDefaultsOnly or VisibleAnywhere for components
	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	class UBoxComponent* MyBoxComponent;

	//Takes the item to represent and creates the pickup from it. Done on BeginPlay and when a player drops an item on the ground.
	void InitializePickup(const TSubclassOf<class UItem> ItemClass, const int32 Quantity);

	/**Align pickups rotation with ground rotation. */
	UFUNCTION(BlueprintImplementableEvent)
	void AlignWithGround();

	//This is used as a template to create the pickup when spawned in
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	class UItem* ItemTemplate;

	//Called when a player takes the pickup
	//Moved from protected to puclic to call in character class
	UFUNCTION()
	void OnTakePickup(class ASurvivalCharacter* Taker);
protected:

	//The item that will be added to the inventory when this pickup is taken
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, ReplicatedUsing = OnRep_Item)
	class UItem* Item;

	UFUNCTION()
	void OnRep_Item();

	/**If some property on the item is modified, we bind this to OnItemModified and refresh the UI if the item gets modified.*/
	UFUNCTION()
	void OnItemModified();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif 

	////Called when a player takes the pickup
	//UFUNCTION()
	//void OnTakePickup(class ASurvivalCharacter* Taker);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
	class UStaticMeshComponent* PickupMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	class UInteractionComponent* InteractionComponent;
};
