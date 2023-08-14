//Extended verison 2023 By [Yalenka] - SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "../Utilities/Loggers/DebugUtilityLogger.h"
#include "../Utilities/Structures/ST_WalkSpeed.h"
#include "../Utilities/Structures/ST_MoveTimeDelay.h"
#include "../Utilities/Structures/ST_CameraHeight.h"

#include "SurvivalCharacter.generated.h"

USTRUCT()
struct FInteractionData
{
	GENERATED_BODY()

	FInteractionData()
	{
		ViewedInteractionComponent = nullptr;
		LastInteractionCheckTime = 0.f;
		bInteractHeld = false;
	}

	//The current interactable component we're viewing, if there is one
	UPROPERTY()
	class UInteractionComponent* ViewedInteractionComponent;

	//The time when we last checked for an interactable
	UPROPERTY()
	float LastInteractionCheckTime;

	//Whether the local player is holding the interact key
	UPROPERTY()
	bool bInteractHeld;

};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquippedItemsChanged, const EEquippableSlot, Slot, const UEquippableItem*, Item);

UCLASS()
class SURVIVALGAME_API ASurvivalCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASurvivalCharacter();

	//The mesh to have equipped if we dont have an item equipped - ie the bare skin meshes
	UPROPERTY(BlueprintReadWrite, Category = Mesh)
	TMap<EEquippableSlot, USkeletalMesh*> NakedMeshes;

	//The players body meshes.
	UPROPERTY(BlueprintReadWrite, BlueprintReadWrite, Category = Mesh)
	TMap<EEquippableSlot, USkeletalMeshComponent*> PlayerMeshes;

	/**Our players inventory*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class UInventoryComponent* PlayerInventory;

	/**Interaction component used to allow other players to loot us when we have died*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class UInteractionComponent* LootPlayerInteraction;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class UCameraComponent* CameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class USkeletalMeshComponent* HelmetMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class USkeletalMeshComponent* ChestMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class USkeletalMeshComponent* LegsMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class USkeletalMeshComponent* FeetMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class USkeletalMeshComponent* VestMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class USkeletalMeshComponent* HandsMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class USkeletalMeshComponent* BackpackMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class USceneCaptureComponent2D* InventoryPlayerCapture;

protected:
	// Called when the game starts or when spawned	
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;
	virtual void Restart() override;
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void SetActorHiddenInGame(bool bNewHidden) override;

public:

	UFUNCTION(BlueprintCallable)
	void SetLootSource(class UInventoryComponent* NewLootSource);

	UFUNCTION(BlueprintPure, Category = "Looting")
	bool IsLooting() const;

protected:

	//Begin being looted by a player
	UFUNCTION()
	void BeginLootingPlayer(class ASurvivalCharacter* Character);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void ServerSetLootSource(class UInventoryComponent* NewLootSource);

	/**The inventory that we are currently looting from. */
	UPROPERTY(ReplicatedUsing = OnRep_LootSource, BlueprintReadWrite)
	UInventoryComponent* LootSource;

	UFUNCTION()
	void OnLootSourceOwnerDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void OnRep_LootSource();

public:

	UFUNCTION(BlueprintCallable, Category = "Looting")
	void LootItem(class UItem* ItemToGive);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLootItem(class UItem* ItemToLoot);



	//How often in seconds to check for an interactable object. Set this to zero if you want to check every tick.
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionCheckFrequency;

	//How far we'll trace when we check if the player is looking at an interactable object
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionCheckDistance;

	void PerformInteractionCheck();

	void CouldntFindInteractable();
	void FoundNewInteractable(UInteractionComponent* Interactable);

	void BeginInteract();
	void EndInteract();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerBeginInteract();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEndInteract();

	void Interact();

	//Information about the current state of the players interaction
	UPROPERTY()
	FInteractionData InteractionData;

	//Helper function to make grabbing interactable faster
	FORCEINLINE class UInteractionComponent* GetInteractable() const { return InteractionData.ViewedInteractionComponent; }

	FTimerHandle TimerHandle_Interact;

public:

	//True if we're interacting with an item that has an interaction time (for example a lamp that takes 2 seconds to turn on)
	bool IsInteracting() const;

	//Get the time till we interact with the current interactable
	float GetRemainingInteractTime() const;


	//Items

/**[Server] Use an item from our inventory.*/
	UFUNCTION(BlueprintCallable, Category = "Items")
	void UseItem(class UItem* Item);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerUseItem(class UItem* Item);

	/**[Server] Drop an item*/
	UFUNCTION(BlueprintCallable, Category = "Items")
	void DropItem(class UItem* Item, const int32 Quantity);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDropItem(class UItem* Item, const int32 Quantity);

	UFUNCTION()
	void ItemAddedToInventory(class UItem* Item);

	UFUNCTION()
	void ItemRemovedFromInventory(class UItem* Item);

	/**needed this because the pickups use a blueprint base class*/
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TSubclassOf<class APickup> PickupClass;

public:

	/**Handle equipping an equippable item*/
	bool EquipItem(class UEquippableItem* Item);
	bool UnEquipItem(class UEquippableItem* Item);

	/**These should never be called directly - UGearItem and UWeaponItem call these on top of EquipItem*/
	void EquipGear(class UGearItem* Gear);
	void UnEquipGear(const EEquippableSlot Slot);
	void EquipWeapon(class UWeaponItem* WeaponItem);
	void UnEquipWeapon();

	/**Called to update the inventory*/
	UPROPERTY(BlueprintAssignable, Category = "Items")
	FOnEquippedItemsChanged OnEquippedItemsChanged;

	/**Return the skeletal mesh component for a given slot*/
	class USkeletalMeshComponent* GetSlotSkeletalMeshComponent(const EEquippableSlot Slot);

	UFUNCTION(BlueprintPure)
	FORCEINLINE TMap<EEquippableSlot, UEquippableItem*> GetEquippedItems() const { return EquippedItems; }

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	FORCEINLINE class AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

protected:

	UFUNCTION(Server, Reliable)
	void ServerUseThrowable();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayThrowableTossFX(class UAnimMontage* MontageToPlay);

	class UThrowableItem* GetThrowable() const;
	void UseThrowable();
	void SpawnThrowable();
	bool CanUseThrowable() const;

	//Allows for efficient access of equipped items
	UPROPERTY(VisibleAnywhere, Category = "Items")
	TMap<EEquippableSlot, UEquippableItem*> EquippedItems;

	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadWrite, Category = "Health")
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Health")
	float MaxHealth;

public:

	//Modify the players health by either a negative or positive amount. Return the amount of health actually removed
	float ModifyHealth(const float Delta);

	UFUNCTION()
	void OnRep_Health(float OldHealth);

	UFUNCTION(BlueprintImplementableEvent)
	void OnHealthModified(const float HealthDelta);

protected:

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_EquippedWeapon)
	class AWeapon* EquippedWeapon;

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void StartFire();
	void StopFire();

	void BeginMeleeAttack();

	UFUNCTION(Server, Reliable)
	void ServerProcessMeleeHit(const FHitResult& MeleeHit);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayMeleeFX();

	UPROPERTY()
	float LastMeleeAttackTime;

	UPROPERTY(EditDefaultsOnly, Category = Melee)
	float MeleeAttackDistance;

	UPROPERTY(EditDefaultsOnly, Category = Melee)
	float MeleeAttackDamage;

	UPROPERTY(EditDefaultsOnly, Category = Melee)
	class UAnimMontage* MeleeAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = Melee)
	TArray<UAnimMontage*> AnimMontages;
	//class UAnimMontage* AnimMontages[3];


	//Called when killed by the player, or killed by something else like the environment
	void Suicide(struct FDamageEvent const& DamageEvent, const AActor* DamageCauser);
	void KilledByPlayer(struct FDamageEvent const& DamageEvent, class ASurvivalCharacter* Character, const AActor* DamageCauser);

	UPROPERTY(ReplicatedUsing = OnRep_Killer)
	class ASurvivalCharacter* Killer;

	UFUNCTION()
	void OnRep_Killer();

	UFUNCTION(BlueprintImplementableEvent)
	void OnDeath();

public:

	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float SprintSpeed;

	UPROPERTY()
	float WalkSpeed;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = Movement)
	bool bSprinting;

	bool CanSprint() const;

	//[local] start and stop sprinting functions
	void StartSprinting();
	void StopSprinting();

	/**[server + local] set sprinting*/
	void SetSprinting(const bool bNewSprinting);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetSprinting(const bool bNewSprinting);

	void StartCrouching();
	void StopCrouching();

	void MoveForward(float Val);
	void MoveRight(float Val);

public:	

	void StartReload();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsAlive() const { return Killer == nullptr; };

	UFUNCTION(BlueprintPure, Category = "Weapons")
	FORCEINLINE bool IsAiming() const { return bIsAiming; }


	public:
	/*In case if we are picking an item using the UI**/
	UFUNCTION(BlueprintCallable)
	void TryPickupItem(APickup* ItemToPickup);

	UFUNCTION(Server, Reliable)
	void ServerPickupItem(APickup* ItemToPickup);

public:
	void ExecuteMove(float AxisValue, bool bForward);
	void MovingOnTheGround(bool bForward, float AxisValue, FRotator ControllerRotation);
	void UpdateWalkSpeed();
	void AppendCharacterState(bool bWithGun, FString Current, FString Target);
	void HelperEnableDisableMove();
	bool CanAim() const;
	void StartAiming();
	void StopAiming();
	void SetAiming(const bool bNewAiming);
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(const bool bNewAiming);
	FORCEINLINE bool GetAiming() const { return bIsAiming; }
	FORCEINLINE void SetPlayingMontage(bool NewValue) { bPlayingMontage = NewValue; }
	FORCEINLINE bool GetPlayingMontage() const { return bPlayingMontage; }

	FORCEINLINE void SetHoldAiming(bool NewValue) { bHoldAiming = NewValue; }
	FORCEINLINE bool GetHoldAiming() const { return bHoldAiming; }

	FORCEINLINE void SetMoveForwardAxis(float NewValue) { MoveForwardAxis = NewValue; }
	FORCEINLINE float GetMoveForwardAxis() const { return MoveForwardAxis; }
	FORCEINLINE void SetMoveRightAxis(float NewValue) { MoveRightAxis = NewValue; }
	FORCEINLINE float GetMoveRightAxis() const { return MoveRightAxis; }
	
	FORCEINLINE void SetWalkPressed(bool NewValue) { bWalkPressed = NewValue; }
	FORCEINLINE bool GetWalkPressed() const { return bWalkPressed; }

	FORCEINLINE void SetProne(bool NewValue) { bProning = NewValue; }
	FORCEINLINE bool GetProne() const { return bProning; }
	FORCEINLINE void SetCrouching(bool NewValue) { bCrouching = NewValue; }
	FORCEINLINE bool GetCrouching() const { return bCrouching; }
	FORCEINLINE void SetHoldWeapon(bool NewValue) { bHoldWeapon = NewValue; }
	FORCEINLINE bool GetHoldWeapon() const { return bHoldWeapon; }

	FORCEINLINE class USpringArmComponent* GetSpringArm() const { return SpringArmComponent; }
	FORCEINLINE void SetAltPressed(bool NewValue) { bAltPressed = NewValue; }
	FORCEINLINE bool GetAltPressed() const { return bAltPressed; }
	FORCEINLINE void SetAltPressedrotation(FRotator NewValue) { AltPressedRotation = NewValue; }
	FORCEINLINE FRotator GetAltPressedRotation() const { return AltPressedRotation; }
	FORCEINLINE void SetAltReleasedRotation(FRotator NewValue) { AltReleasedRotation = NewValue; }
	FORCEINLINE FRotator GetAltReleasedRotation() const { return AltReleasedRotation; }

	FORCEINLINE void SetRunPressed(bool NewValue) { bRunPressed = NewValue; }
	FORCEINLINE bool GetRunPressed() const { return bRunPressed; }

	FORCEINLINE void SetWalkSpeed(float NewValue) { WalkSpeed = NewValue; }
	FORCEINLINE float GetWalkSpeed() const { return WalkSpeed; }

	FORCEINLINE void SetEnableMove(bool NewValue) { bEnableMove = NewValue; }
	FORCEINLINE bool GetEnableMove() const { return bEnableMove; }

	FORCEINLINE void SetDead(bool NewValue) { bDead = NewValue; }
	FORCEINLINE bool GetDead() const { return bDead; }

protected:
	UPROPERTY(Transient, Replicated)
	bool bIsAiming;
	UPROPERTY(Transient, Replicated)
	bool bHoldAiming;
	bool bCrouching;
	float MoveForwardAxis;
	bool bWalkPressed;
	bool bRunPressed;
	bool bPlayingMontage;
	bool bProning;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterState|TPP")
	bool bDead;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "CharacterState|TPP")
	bool bHoldWeapon;
	float MoveRightAxis;
	bool bAltPressed;
	FRotator AltPressedRotation;
	FRotator AltReleasedRotation;
	FRotator AltControllerRotation;
	bool bEnableMove = true;

public:
	/**
	* Replication of Looking-UpDown and Looking-LeftRight
	*/
	void LookUp(float Pitch);
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSet_PlayerPitch(float Pitch);
	UPROPERTY(Replicated)
	float PlayerPitch;

	void Turn(float Yaw);
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSet_PlayerYaw(float Yaw);
	UPROPERTY(Replicated)
	float PlayerYaw;

};
