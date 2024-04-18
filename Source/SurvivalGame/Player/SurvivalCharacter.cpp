//Extended verison 2023 By [Yalenka] - SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020


#include "SurvivalCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "../Player/SurvivalPlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Components/InventoryComponent.h"
#include "../Components/InteractionComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "../Items/EquippableItem.h"
#include "../Items/GearItem.h"
#include "../Items/AccItem.h"
#include "../Items/WeaponItem.h"
#include "../Items/ThrowableItem.h"
#include "Materials/MaterialInstance.h"
#include "../World/Pickup.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/DamageType.h"
#include "../Weapons/SurvivalDamageTypes.h"
#include "../Weapons/ThrowableWeapon.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "../SurvivalGame.h"
#include "../Weapons/Weapon.h"
#include "../Widgets/SurvivalHUD.h"




#include "Engine/GameEngine.h"
#include "SurvivalAnimInstance.h"

#define LOCTEXT_NAMESPACE "SurvivalCharacter"

static FName NAME_AimDownSightsSocket("ADSSocket");

// Sets default values
ASurvivalCharacter::ASurvivalCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup Settings and Attachments of SpringArm and TPP Camera Components
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent/*, FName("camera_tppSocket")*/);
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->SetRelativeLocation(FVector(0.0f, 25.0f, 100.0f));
	SpringArmComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	SpringArmComponent->TargetArmLength = 200.0f;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->bUsePawnControlRotation = false;

	HelmetMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Helmet, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HelmetMesh")));
	ChestMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Chest, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ChestMesh")));
	LegsMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Legs, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LegsMesh")));
	FeetMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Feet, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FeetMesh")));
	VestMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Vest, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VestMesh")));
	HandsMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Hands, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HandsMesh")));
	BackpackMesh = PlayerMeshes.Add(EEquippableSlot::EIS_Backpack, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BackpackMesh")));

	//Tell all the body meshes to use the head mesh for animation
	for (auto& Kvp : PlayerMeshes)
	{
		USkeletalMeshComponent* MeshComponent = Kvp.Value;
		MeshComponent->SetupAttachment(GetMesh());
		MeshComponent->SetMasterPoseComponent(GetMesh());
	}

	PlayerMeshes.Add(EEquippableSlot::EIS_Head, GetMesh());
	GetMesh()->SetOwnerNoSee(true);

	//Give the player an inventory with 20 slots, and an 80kg capacity
	PlayerInventory = CreateDefaultSubobject<UInventoryComponent>("PlayerInventory");
	PlayerInventory->SetCapacity(20);
	PlayerInventory->SetWeightCapacity(80.f);
	PlayerInventory->OnItemAdded.AddDynamic(this, &ASurvivalCharacter::ItemAddedToInventory);
	PlayerInventory->OnItemRemoved.AddDynamic(this, &ASurvivalCharacter::ItemRemovedFromInventory);

	LootPlayerInteraction = CreateDefaultSubobject<UInteractionComponent>("PlayerInteraction");
	LootPlayerInteraction->InteractableActionText = LOCTEXT("LootPlayerText", "Loot");
	LootPlayerInteraction->InteractableNameText = LOCTEXT("LootPlayerName", "Player");
	LootPlayerInteraction->SetupAttachment(GetRootComponent());
	LootPlayerInteraction->SetActive(false, true);
	LootPlayerInteraction->bAutoActivate = false;

	//We create a scene component that the inventory camera roots to. This way we can rotate this root to allow players to rotate their character in the inventory menu.
	USceneComponent* InventoryCaptureRoot = CreateDefaultSubobject<USceneComponent>("InventoryCaptureRoot");
	InventoryCaptureRoot->SetupAttachment(GetCapsuleComponent());
	InventoryCaptureRoot->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));

	InventoryPlayerCapture = CreateDefaultSubobject<USceneCaptureComponent2D>("InventoryPlayerCapture");
	InventoryPlayerCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	InventoryPlayerCapture->SetupAttachment(InventoryCaptureRoot);
	InventoryPlayerCapture->SetRelativeLocation(FVector(-1100.f, 0.f, 0.f));

	InteractionCheckFrequency = 0.f;
	InteractionCheckDistance = 1000.f;

	MaxHealth = 100.f;
	Health = MaxHealth;

	MeleeAttackDistance = 150.f;
	MeleeAttackDamage = 20.f;

	SprintSpeed = GetCharacterMovement()->MaxWalkSpeed * 1.3f;
	WalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	bIsAiming = false;

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	SetReplicateMovement(true);
	SetReplicates(true);
	bAlwaysRelevant = true;
}

// Called when the game starts or when spawned
void ASurvivalCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (!IsLocallyControlled())
	{
		CameraComponent->DestroyComponent();
		SpringArmComponent->DestroyComponent();
		InventoryPlayerCapture->DestroyComponent();
	}

	if (InventoryPlayerCapture)
	{
		InventoryPlayerCapture->ShowOnlyActors.Add(this);
	}

	LootPlayerInteraction->OnInteract.AddDynamic(this, &ASurvivalCharacter::BeginLootingPlayer);

	//Try to display the players platform name on their loot card
	if (APlayerState* PS = GetPlayerState())
	{
		LootPlayerInteraction->SetInteractableNameText(FText::FromString(PS->GetPlayerName()));
	}

	//When the player spawns in they have no items equipped, so cache these items (That way, if a player unequips an item we can set the mesh back to the naked character)
	for (auto& PlayerMesh : PlayerMeshes)
	{
		NakedMeshes.Add(PlayerMesh.Key, PlayerMesh.Value->SkeletalMesh);
	}

	MyAnimInstance = Cast<USurvivalAnimInstance>(GetMesh()->GetAnimInstance());
	if (MyAnimInstance != nullptr)
	{
		GetMesh()->GetAnimInstance()->OnMontageEnded.AddDynamic(this, &ASurvivalCharacter::Event_OnMontageEnded);
		MyAnimInstance->OnUnEquip.AddDynamic(this, &ASurvivalCharacter::Event_OnUnEquip);
		MyAnimInstance->OnEquip.AddDynamic(this, &ASurvivalCharacter::Event_OnEquip);
	}
}

void ASurvivalCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASurvivalCharacter, bSprinting);
	DOREPLIFETIME(ASurvivalCharacter, EquippedWeapon);
	DOREPLIFETIME(ASurvivalCharacter, WeaponOne_1);
	DOREPLIFETIME(ASurvivalCharacter, WeaponTwo_2);
	DOREPLIFETIME(ASurvivalCharacter, ReadyEquipWeapon);
	DOREPLIFETIME(ASurvivalCharacter, AccOwnerWeapon);
	DOREPLIFETIME(ASurvivalCharacter, bIsPlayingMontage);
	DOREPLIFETIME(ASurvivalCharacter, Killer);
	DOREPLIFETIME(ASurvivalCharacter, ItemsInRange);

	DOREPLIFETIME_CONDITION(ASurvivalCharacter, LootSource, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ASurvivalCharacter, Health, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ASurvivalCharacter, bIsAiming, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ASurvivalCharacter, PlayerPitch, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ASurvivalCharacter, PlayerYaw, COND_SkipOwner);

}

bool ASurvivalCharacter::IsInteracting() const
{
	return GetWorldTimerManager().IsTimerActive(TimerHandle_Interact);
}

float ASurvivalCharacter::GetRemainingInteractTime() const
{
	return GetWorldTimerManager().GetTimerRemaining(TimerHandle_Interact);
}

void ASurvivalCharacter::UseItem(class UItem* Item)
{
	if (!HasAuthority() && Item)
	{
		ServerUseItem(Item);
	}

	if (HasAuthority())
	{
		if (PlayerInventory && !PlayerInventory->FindItem(Item))
		{
			return;
		}
	}

	if (Item)
	{
		Item->OnUse(this);
		Item->Use(this);
	}
}

void ASurvivalCharacter::DropItem(class UItem* Item, const int32 Quantity)
{
	if (PlayerInventory && Item && PlayerInventory->FindItem(Item))
	{
		if (!HasAuthority())
		{
			ServerDropItem(Item, Quantity);
			return;
		}

		if (HasAuthority())
		{
			const int32 ItemQuantity = Item->GetQuantity();
			const int32 DroppedQuantity = PlayerInventory->ConsumeItem(Item, Quantity);
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.bNoFail = true;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			FVector SpawnLocation = GetActorLocation();
			SpawnLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

			FTransform SpawnTransform(GetActorRotation(), SpawnLocation);

			//ensure(PickupClass);
			if (IsValid(PickupClass))
			{
				if (APickup* Pickup = GetWorld()->SpawnActor<APickup>(PickupClass, SpawnTransform, SpawnParams))
				{
					Pickup->InitializePickup(Item->GetClass(), DroppedQuantity);
				}
			}
			else
			{
				print("Pickup Not valid to spawn: @ASurvivalCharacter::DropItem");
				return;
			}
		}
	}
}

void ASurvivalCharacter::DiscardWeapon(class AWeapon* WeaponActor, int32 Quantity)
{
	if (IsValid(WeaponActor))
	{
		if (WeaponActor->bIsOnHand)
		{
			DropItem(WeaponActor->Item, Quantity);

			//GetEquippedWeapon()->OnUnEquip();
			//GetEquippedWeapon()->Destroy();
			//SetEquippedWeapon(nullptr);
			//OnRep_EquippedWeapon();
		}

		if (GetIsHoldWeapon())
		{
			DropItem(WeaponActor->Item, Quantity);
		}
		else {
			switch (WeaponActor->Position) {
			case EWeaponPosition::E_Left:
				if (GetPrimaryWeapon())
				{
					DropItem(WeaponActor->Item, Quantity);

					//GetPrimaryWeapon()->OnUnEquip();
					//GetPrimaryWeapon()->Destroy();
					//SetWeaponOne_1(nullptr);
					//OnRep_WeapnOne_1();
				}
				break;
			case EWeaponPosition::E_Right:
				if (GetSecondaryWeapon())
				{
					DropItem(WeaponActor->Item, Quantity);

					//GetSecondaryWeapon()->OnUnEquip();
					//GetSecondaryWeapon()->Destroy();
					//SetWeaponOne_2(nullptr);
					//OnRep_WeapnOne_2();
				}
				break;
			case EWeaponPosition::E_MAX:

				break;
			default:
				break;
			}
		}
		WeaponActor->Destroy();
	}
}

void ASurvivalCharacter::ItemAddedToInventory(class UItem* Item)
{

}

void ASurvivalCharacter::ItemRemovedFromInventory(class UItem* Item)
{

}

bool ASurvivalCharacter::EquipItem(class UEquippableItem* Item)
{
	/*EquippedItems.Add(Item->Slot, Item);
	OnEquippedItemsChanged.Broadcast(Item->Slot, Item);
	return true;*/

	if (Item)
	{
		if (Item->ItemType != EItemType::E_Accessories)
		{
			EquippedItems.Add(Item->Slot, Item);
			OnEquippedItemsChanged.Broadcast(Item->Slot, Item);
			return true;
		}
		else
		{
			if (Item->ItemType == EItemType::E_Accessories)
			{
				switch (AccOwnerWeapon->Position)
				{
				default:
					break;
				case EWeaponPosition::E_Left:
				{
					PrimaryWeaponEquippedItems.Add(Item->Slot, Item);
					OnEquippedItemsChanged.Broadcast(Item->Slot, Item);
					return true;
				}
				break;
				case EWeaponPosition::E_Right:
				{
					SecondaryWeaponEquippedItems.Add(Item->Slot, Item);
					OnEquippedItemsChanged.Broadcast(Item->Slot, Item);
					return true;
				}
				break;
				}
			}
		}
	}
	return false;
}

bool ASurvivalCharacter::UnEquipItem(class UEquippableItem* Item)
{
	if (Item)
	{
		if (EquippedItems.Contains(Item->Slot))
		{
			if (Item == *EquippedItems.Find(Item->Slot))
			{
				EquippedItems.Remove(Item->Slot);
				OnEquippedItemsChanged.Broadcast(Item->Slot, nullptr);
				return true;
			}
		}
	}
	return false;
}

void ASurvivalCharacter::EquipGear(class UGearItem* Gear)
{
	if (USkeletalMeshComponent* GearMesh = GetSlotSkeletalMeshComponent(Gear->Slot))
	{
		GearMesh->SetSkeletalMesh(Gear->Mesh);
		GearMesh->SetMaterial(GearMesh->GetMaterials().Num() - 1, Gear->MaterialInstance);
	}
}

void ASurvivalCharacter::UnEquipGear(const EEquippableSlot Slot)
{
	if (USkeletalMeshComponent* EquippableMesh = GetSlotSkeletalMeshComponent(Slot))
	{
		if (USkeletalMesh* BodyMesh = *NakedMeshes.Find(Slot))
		{
			EquippableMesh->SetSkeletalMesh(BodyMesh);

			//Put the materials back on the body mesh (Since gear may have applied a different material)
			for (int32 i = 0; i < BodyMesh->Materials.Num(); ++i)
			{
				if (BodyMesh->Materials.IsValidIndex(i))
				{
					EquippableMesh->SetMaterial(i, BodyMesh->Materials[i].MaterialInterface);
				}
			}
		}
		else
		{
			//For some gear like backpacks, there is no naked mesh
			EquippableMesh->SetSkeletalMesh(nullptr);
		}
	}
}

void ASurvivalCharacter::EquipWeapon(class UWeaponItem* WeaponItem)
{
	if (WeaponItem && WeaponItem->WeaponClass && HasAuthority())
	{
		//if (EquippedWeapon)
		//{
		//	UnEquipWeapon();
		//}

		////Spawn the weapon in
		//FActorSpawnParameters SpawnParams;
		//SpawnParams.bNoFail = true;
		//SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		//SpawnParams.Owner = SpawnParams.Instigator = this;

		//if (AWeapon* Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponItem->WeaponClass, SpawnParams))
		//{
		//	Weapon->Item = WeaponItem;

		//	EquippedWeapon = Weapon;
		//	OnRep_EquippedWeapon();

		//	Weapon->OnEquip();
		//}

		
		PickupWeapon(WeaponItem, false, EWeaponPosition::E_Left);
	}
}

void ASurvivalCharacter::UnEquipWeapon()
{
	if (HasAuthority() && EquippedWeapon)
	{
		//EquippedWeapon->OnUnEquip();
		//EquippedWeapon->Destroy();
		//EquippedWeapon = nullptr;
		//OnRep_EquippedWeapon();
	}
}

class USkeletalMeshComponent* ASurvivalCharacter::GetSlotSkeletalMeshComponent(const EEquippableSlot Slot)
{
	if (PlayerMeshes.Contains(Slot))
	{
		return *PlayerMeshes.Find(Slot);
	}
	return nullptr;
}

void ASurvivalCharacter::SetHoldWeapon(AWeapon* WeaponToSet)
{
	EquippedWeapon = WeaponToSet;
	if (IsValid(GetHoldWeapon())) {
		OnWeaponChanged.Broadcast(EquippedWeapon, EquippedWeapon->Position, true);
	}
	else
	{
		OnWeaponChanged.Broadcast(GetHoldWeapon(), EWeaponPosition::E_Left, true);
	}
}

void ASurvivalCharacter::SetPrimaryWeapon(AWeapon* WeaponToSet)
{
	WeaponOne_1 = WeaponToSet;
	OnWeaponChanged.Broadcast(WeaponOne_1, EWeaponPosition::E_Left, 0);
}

void ASurvivalCharacter::SetSecondaryWeapon(AWeapon* WeaponToSet)
{
	WeaponTwo_2 = WeaponToSet;
	OnWeaponChanged.Broadcast(WeaponTwo_2, EWeaponPosition::E_Right, 0);
}

class UThrowableItem* ASurvivalCharacter::GetThrowable() const
{
	UThrowableItem* EquippedThrowable = nullptr;

	if (EquippedItems.Contains(EEquippableSlot::EIS_Throwable))
	{
		EquippedThrowable = Cast<UThrowableItem>(*EquippedItems.Find(EEquippableSlot::EIS_Throwable));
	}

	return EquippedThrowable;
}

void ASurvivalCharacter::UseThrowable()
{
	if (CanUseThrowable())
	{
		if (UThrowableItem* Throwable = GetThrowable())
		{
			if (HasAuthority())
			{
				SpawnThrowable();

				if (PlayerInventory)
				{
					PlayerInventory->ConsumeItem(Throwable, 1);
				}
			}
			else
			{
				if (Throwable->GetQuantity() <= 1)
				{
					EquippedItems.Remove(EEquippableSlot::EIS_Throwable);
					OnEquippedItemsChanged.Broadcast(EEquippableSlot::EIS_Throwable, nullptr);
				}

				//Locally play grenade throw instantly - by the time server spawns the grenade in the throw animation should roughly sync up with the spawning of the grenade
				PlayAnimMontage(Throwable->ThrowableTossAnimation);
				ServerUseThrowable();
			}
		}
	}
}

void ASurvivalCharacter::SpawnThrowable()
{
	if (HasAuthority())
	{
		if (UThrowableItem* CurrentThrowable = GetThrowable())
		{
			if (CurrentThrowable->ThrowableClass)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = SpawnParams.Instigator = this;
				SpawnParams.bNoFail = true;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				FVector EyesLoc;
				FRotator EyesRot;

				GetController()->GetPlayerViewPoint(EyesLoc, EyesRot);

				//Spawn throwable slightly in front of our face so it doesnt collide with our player
				EyesLoc = (EyesRot.Vector() * 20.f) + EyesLoc;

				if (AThrowableWeapon* ThrowableWeapon = GetWorld()->SpawnActor<AThrowableWeapon>(CurrentThrowable->ThrowableClass, FTransform(EyesRot, EyesLoc), SpawnParams))
				{
					MulticastPlayThrowableTossFX(CurrentThrowable->ThrowableTossAnimation);
				}
			}
		}
	}
}

bool ASurvivalCharacter::CanUseThrowable() const
{
	return GetThrowable() != nullptr && GetThrowable()->ThrowableClass != nullptr;
}

void ASurvivalCharacter::MulticastPlayThrowableTossFX_Implementation(UAnimMontage* MontageToPlay)
{
	//Local player already instantly played grenade throw anim
	if (GetNetMode() != NM_DedicatedServer && !IsLocallyControlled())
	{
		PlayAnimMontage(MontageToPlay);
	}
}

void ASurvivalCharacter::ServerUseThrowable_Implementation()
{
	UseThrowable();
}

void ASurvivalCharacter::ServerDropItem_Implementation(class UItem* Item, const int32 Quantity)
{
	DropItem(Item, Quantity);
}

bool ASurvivalCharacter::ServerDropItem_Validate(class UItem* Item, const int32 Quantity)
{
	return true;
}

void ASurvivalCharacter::ServerUseItem_Implementation(class UItem* Item)
{
	UseItem(Item);
}

bool ASurvivalCharacter::ServerUseItem_Validate(class UItem* Item)
{
	return true;
}

float ASurvivalCharacter::ModifyHealth(const float Delta)
{
	const float OldHealth = Health;

	Health = FMath::Clamp<float>(Health + Delta, 0.f, MaxHealth);

	return Health - OldHealth;
}

void ASurvivalCharacter::OnRep_Health(float OldHealth)
{
	OnHealthModified(Health - OldHealth);
}

void ASurvivalCharacter::OnRep_EquippedWeapon()
{
	if (GetHoldWeapon())
	{
		GetHoldWeapon()->OnEquip();
		SetIsHoldWeapon(true);
	}
	else
	{
		SetIsHoldWeapon(false);
	}
}

void ASurvivalCharacter::OnRep_PrimaryWeapon()
{
	if (GetPrimaryWeapon())
	{
		GetPrimaryWeapon()->OnEquip();
	}
	else
	{
		SetPrimaryWeapon(nullptr);
	}
}

void ASurvivalCharacter::OnRep_SecondaryWeapon()
{
	if (GetSecondaryWeapon())
	{
		GetSecondaryWeapon()->OnEquip();
	}
	else
	{
		SetSecondaryWeapon(nullptr);
	}
}

void ASurvivalCharacter::StartFire()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StartFire();
	}
	else
	{
		BeginMeleeAttack();
	}
}

void ASurvivalCharacter::StopFire()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StopFire();
	}
}

void ASurvivalCharacter::BeginMeleeAttack()
{
	print("Melee Attack Performed, but no animation montage found for melee attacks");

	if (AnimMontages.Num() > 0)
	{
		if (GetWorld()->TimeSince(LastMeleeAttackTime) > MeleeAttackMontage->GetPlayLength())
		{
			FHitResult Hit;
			FCollisionShape Shape = FCollisionShape::MakeSphere(15.f);

			FVector StartTrace = CameraComponent->GetComponentLocation();
			FVector EndTrace = (CameraComponent->GetComponentRotation().Vector() * MeleeAttackDistance) + StartTrace;

			FCollisionQueryParams QueryParams = FCollisionQueryParams("MeleeSweep", false, this);

			if (!GetMesh()->GetAnimInstance()->Montage_IsPlaying(MeleeAttackMontage))
			{
				//PlayRandom Anim Montage in range
				int32 MontageIndex = FMath::RandRange(0, 2);
				if (AnimMontages.IsValidIndex(MontageIndex))
				{
					MeleeAttackMontage = AnimMontages[MontageIndex];

					if (MeleeAttackMontage)
					{
						PlayAnimMontage(MeleeAttackMontage);
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Montage is playing"));
			}

			if (GetWorld()->SweepSingleByChannel(Hit, StartTrace, EndTrace, FQuat(), COLLISION_WEAPON, Shape, QueryParams))
			{
				if (ASurvivalCharacter* HitPlayer = Cast<ASurvivalCharacter>(Hit.GetActor()))
				{
					if (ASurvivalPlayerController* PC = Cast<ASurvivalPlayerController>(GetController()))
					{
						PC->ClientShotHitConfirmed();
					}
				}
			}

			ServerProcessMeleeHit(Hit);

			LastMeleeAttackTime = GetWorld()->GetTimeSeconds();
		}
	}
}

void ASurvivalCharacter::MulticastPlayMeleeFX_Implementation()
{
	if (!IsLocallyControlled())
	{
		//PlayAnimMontage(MeleeAttackMontage);
		int32 MontageIndex = FMath::RandRange(0, 2);
		if (AnimMontages.IsValidIndex(MontageIndex))
		{
			MeleeAttackMontage = AnimMontages[MontageIndex];

			if (MeleeAttackMontage)
			{
				PlayAnimMontage(MeleeAttackMontage);
			}
		}
	}
}

void ASurvivalCharacter::ServerProcessMeleeHit_Implementation(const FHitResult& MeleeHit)
{
	if (!GetMesh()->GetAnimInstance()->Montage_IsPlaying(MeleeAttackMontage))
	{
		MulticastPlayMeleeFX();
		if (GetWorld()->TimeSince(LastMeleeAttackTime) > MeleeAttackMontage->GetPlayLength() && (GetActorLocation() - MeleeHit.ImpactPoint).Size() <= MeleeAttackDistance)
		{
			UGameplayStatics::ApplyPointDamage(MeleeHit.GetActor(), MeleeAttackDamage, (MeleeHit.TraceStart - MeleeHit.TraceEnd).GetSafeNormal(), MeleeHit, GetController(), this, UMeleeDamage::StaticClass());
			LastMeleeAttackTime = GetWorld()->GetTimeSeconds();
		}
		LastMeleeAttackTime = GetWorld()->GetTimeSeconds();
	}
}

void ASurvivalCharacter::Suicide(struct FDamageEvent const& DamageEvent, const AActor* DamageCauser)
{
	Killer = this;
	OnRep_Killer();
}

void ASurvivalCharacter::KilledByPlayer(struct FDamageEvent const& DamageEvent, class ASurvivalCharacter* Character, const AActor* DamageCauser)
{
	Killer = Character;
	OnRep_Killer();
}

void ASurvivalCharacter::OnRep_Killer()
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	GetMesh()->SetOwnerNoSee(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	SetReplicateMovement(false);

	LootPlayerInteraction->Activate();

	TArray<UEquippableItem*> EquippedInvItems;
	EquippedItems.GenerateValueArray(EquippedInvItems);

	for (auto& Equippable : EquippedInvItems)
	{
		Equippable->SetEquipped(false);
	}
	if (HasAuthority())
	{
		TArray<UEquippableItem*>Equipables;
		EquippedItems.GenerateValueArray(Equipables);
		for (auto& EquippableItem : Equipables)
		{
			if (!IsValid(EquippableItem)) { return; }
			EquippableItem->SetEquipped(false);
		}

		TArray<UEquippableItem*>PrimaryWeaponAccEquippables;
		PrimaryWeaponEquippedItems.GenerateValueArray(PrimaryWeaponAccEquippables);
		for (auto& PrimaryAcccItem : PrimaryWeaponAccEquippables)
		{
			if (!IsValid(PrimaryAcccItem)) { return; }
			PrimaryAcccItem->SetEquipped(false);
		}

		TArray<UEquippableItem*>SecondaryWeaponAccEquippables;
		SecondaryWeaponEquippedItems.GenerateValueArray(SecondaryWeaponAccEquippables);
		for (auto& SecondaryAcccItem : SecondaryWeaponAccEquippables)
		{
			if (!IsValid(SecondaryAcccItem)) { return; }
			SecondaryAcccItem->SetEquipped(false);
		}
	}

	if (IsLocallyControlled())
	{
		SpringArmComponent->TargetArmLength = 500.f;
		SpringArmComponent->AttachToComponent(GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		bUseControllerRotationPitch = true;

		if (ASurvivalPlayerController* PC = Cast<ASurvivalPlayerController>(GetController()))
		{
			PC->Died(Killer);
		}
	}
}

bool ASurvivalCharacter::CanSprint() const
{
	return !IsAiming();
}

void ASurvivalCharacter::StartSprinting()
{
	SetSprinting(true);
}

void ASurvivalCharacter::StopSprinting()
{
	SetSprinting(false);
}

void ASurvivalCharacter::SetSprinting(const bool bNewSprinting)
{
	if ((bNewSprinting && !CanSprint()) || bNewSprinting == bSprinting)
	{
		return;
	}

	if (!HasAuthority())
	{
		ServerSetSprinting(bNewSprinting);
	}

	bSprinting = bNewSprinting;

	GetCharacterMovement()->MaxWalkSpeed = bSprinting ? SprintSpeed : WalkSpeed;
}

void ASurvivalCharacter::ServerSetSprinting_Implementation(const bool bNewSprinting)
{
	SetSprinting(bNewSprinting);
}

bool ASurvivalCharacter::ServerSetSprinting_Validate(const bool bNewSprinting)
{
	return true;
}

void ASurvivalCharacter::StartCrouching()
{
	Crouch();
}

void ASurvivalCharacter::StopCrouching()
{
	UnCrouch();
}

void ASurvivalCharacter::MoveForward(float Val)
{
	if (Val != 0.f)
	{
		AddMovementInput(GetActorForwardVector(), Val);
	}
}

void ASurvivalCharacter::MoveRight(float Val)
{
	if (Val != 0.f)
	{
		AddMovementInput(GetActorRightVector(), Val);
	}
}

bool ASurvivalCharacter::CanAim() const
{
	return EquippedWeapon != nullptr;
}

void ASurvivalCharacter::StartAiming()
{
	if (CanAim())
	{
		SetAiming(true);
	}
}

void ASurvivalCharacter::StopAiming()
{
	SetAiming(false);
}

void ASurvivalCharacter::SetAiming(const bool bNewAiming)
{
	if ((bNewAiming && !CanAim()) || bNewAiming == bIsAiming)
	{
		return;
	}

	if (!HasAuthority())
	{
		ServerSetAiming(bNewAiming);
	}

	bIsAiming = bNewAiming;
}

void ASurvivalCharacter::LookUp(float Pitch)
{
	if (Controller == nullptr) { return; }

	AddControllerPitchInput((Pitch));
	FRotator NormalizedRotation = (GetControlRotation() - GetActorRotation()).GetNormalized();
	//PlayerPitch = NormalizedRotation.Pitch;
	ServerSet_PlayerPitch(NormalizedRotation.Pitch);
}

void ASurvivalCharacter::Turn(float Yaw)
{
	if (Controller == nullptr) { return; }

	AddControllerYawInput(Yaw);
	FRotator NormalizedRotation = (GetControlRotation() - GetActorRotation()).GetNormalized();
	//PlyerYaw = NormalizedRotation.Yaw;
	ServerSet_PlayerYaw(NormalizedRotation.Yaw);
}

void ASurvivalCharacter::PickupWeapon(class UWeaponItem* WeaponItem, bool bIsAssign, EWeaponPosition Position)
{
	if (WeaponItem && WeaponItem->WeaponClass && HasAuthority())
	{
		AWeapon* ReplaceWeapon = nullptr;
		EWeaponPosition TargetPosition;
		bool bTargetIsOnHand{};

		//Assign the position for the weapon
		if (bIsAssign)
		{
			AssignPosition(Position, TargetPosition, bTargetIsOnHand);
		}
		else
		{
			TargetPosition = AutoPosition(bTargetIsOnHand);
		}

		//Get Weapon object that needs to be replaced
		if (bTargetIsOnHand && IsValid(GetHoldWeapon()))
		{
			ReplaceWeapon = GetHoldWeapon();
		}
		else
		{
			switch (TargetPosition)
			{
			case EWeaponPosition::E_Left:
			{
				if (IsValid(GetPrimaryWeapon()))
				{
					ReplaceWeapon = GetPrimaryWeapon();
				}
			}
			break;
			case EWeaponPosition::E_Right:
			{
				if (IsValid(GetSecondaryWeapon()))
				{
					ReplaceWeapon = GetSecondaryWeapon();
				}
			}
			default:
				break;
			}
		}

		if (IsValid(ReplaceWeapon) && IsValid(ReplaceWeapon->Item))
		{
			DiscardWeapon(ReplaceWeapon, ReplaceWeapon->Item->GetQuantity());
			//DropItem(ReplaceWeapon->Item, ReplaceWeapon->Item->GetQuantity());
		}

		//Spawn the weapon in
		FActorSpawnParameters SpawnParams;
		SpawnParams.bNoFail = true;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		SpawnParams.Owner = SpawnParams.Instigator = this;
		SpawnParams.bDeferConstruction = true;

		FTransform Transformx;
		Transformx.SetLocation(FVector::ZeroVector);
		AWeapon* Weapon = nullptr;

		Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponItem->WeaponClass, SpawnParams);
		if (Weapon)
		{
			Weapon->bIsOnHand = bTargetIsOnHand;
			Weapon->Position = TargetPosition;
			Weapon->Item = WeaponItem;
			Weapon->FinishSpawning(Transformx);

			//if (bTargetIsOnHand) 
			if (bTargetIsOnHand)
			{
				SetHoldWeapon(Weapon);
				//GetHoldWeapon()->Item = WeaponItem;
				OnRep_EquippedWeapon();
				GetHoldWeapon()->OnEquip();
			}
			else
			{
				switch (TargetPosition)
				{
				case EWeaponPosition::E_Left:
				{
					SetPrimaryWeapon(Weapon);
					GetPrimaryWeapon()->bIsOnHand = bTargetIsOnHand;
					GetPrimaryWeapon()->Position = TargetPosition;
				}
				break;
				case EWeaponPosition::E_Right:
				{
					SetSecondaryWeapon(Weapon);
					GetSecondaryWeapon()->bIsOnHand = bTargetIsOnHand;
					GetSecondaryWeapon()->Position = TargetPosition;
				}
				break;
				case EWeaponPosition::E_MAX:

					break;
				default:
					break;
				}
			}
		}
	}
}

void ASurvivalCharacter::AssignPosition(const EWeaponPosition& Assign, EWeaponPosition& Position, bool& bIsOnHand)
{
	if (GetHoldWeapon()) {
		if (GetHoldWeapon()->Position == Assign) {
			Position = Assign;
			bIsOnHand = 1;
		}
		else {
			Position = Assign;
			bIsOnHand = 0;
		}
	}
	else {
		Position = Assign;
		bIsOnHand = 0;
	}
}

EWeaponPosition ASurvivalCharacter::AutoPosition(bool& bIsOnHand)
{
	int32 CurrentAmount = 0;
	if (GetPrimaryWeapon())
	{
		//CurrentAmount += 1;
		CurrentAmount++;
	}

	if (GetSecondaryWeapon())
	{
		//CurrentAmount += 1;
		CurrentAmount++;
	}

	if (GetHoldWeapon())
	{
		//CurrentAmount += 1;
		CurrentAmount++;
	}

	if (CurrentAmount == 0)
	{
		bIsOnHand = 1;
		return EWeaponPosition::E_Left;
	}
	else
	{
		if (CurrentAmount < 2) {
			if (GetHoldWeapon()) {
				switch (GetHoldWeapon()->Position) {
				case EWeaponPosition::E_Left:
					bIsOnHand = 0;
					return EWeaponPosition::E_Right;
					break;
				case EWeaponPosition::E_Right:
					bIsOnHand = 0;
					return EWeaponPosition::E_Left;
					break;
				case EWeaponPosition::E_MAX:
					break;
				default:
					break;
				}
			}
			else {
				if (GetPrimaryWeapon()) {
					bIsOnHand = 0;
					return EWeaponPosition::E_Right;

				}
				else {
					bIsOnHand = 0;
					return EWeaponPosition::E_Left;
				}
			}
		}
		else {
			if (GetHoldWeapon()) {
				switch (GetHoldWeapon()->Position) {
				case EWeaponPosition::E_Left:
					bIsOnHand = 1;
					return EWeaponPosition::E_Left;

					break;
				case EWeaponPosition::E_Right:
					bIsOnHand = 1;
					return EWeaponPosition::E_Right;

					break;
				case EWeaponPosition::E_MAX:
					break;
				default:
					break;
				}
			}
			else {
				bIsOnHand = 0;
				return EWeaponPosition::E_Left;

			}
		}
	}

	return EWeaponPosition::E_MAX;
}

void ASurvivalCharacter::SwitchToPrimaryWeapon()
{
	if (HasAuthority())
	{
		if (IsValid(GetPrimaryWeapon()))
		{
			if (!bIsPlayingMontage)
			{
				ReadyEquipWeapon = GetPrimaryWeapon();

				if (IsValid(GetHoldWeapon()))
				{
					PlayMontage(EMontageType::EIS_UnEquip, 0);
				}
				else
				{
					PlayMontage(EMontageType::EIS_Equip, 0);
				}
			}
		}
	}
	else
	{
		Server_SwitchToPrimaryWeapon();
	}
}

void ASurvivalCharacter::Server_SwitchToPrimaryWeapon_Implementation()
{
	SwitchToPrimaryWeapon();
}

void ASurvivalCharacter::SwitchToSecondaryWeapon()
{
	if (HasAuthority())
	{
		if (IsValid(GetSecondaryWeapon()))
		{
			if (!bIsPlayingMontage)
			{
				ReadyEquipWeapon = GetSecondaryWeapon();

				if (IsValid(GetHoldWeapon()))
				{
					PlayMontage(EMontageType::EIS_UnEquip, 0);
				}
				else
				{
					PlayMontage(EMontageType::EIS_Equip, 0);
				}
			}
		}
	}
	else
	{
		Server_SwitchToSecondaryWeapon();
	}
}

void ASurvivalCharacter::EquipAcc(class UAccItem* Acc)
{
	if (AccOwnerWeapon)
	{
		if (UStaticMeshComponent* Accmesh = AccOwnerWeapon->GetSlotStaticmeshComponents(Acc->Slot))
		{
			switch (Acc->Slot)
			{
			default:
				break;
			case EEquippableSlot::EIS_Mag:
			{
				AccOwnerWeapon->UpdateMag(Acc, Accmesh);
			}
			break;
			}
		}
	}
}

void ASurvivalCharacter::UnEquipAcc(const EEquippableSlot Slot, class UAccItem* Acc)
{
	if (UStaticMeshComponent* AccMesh = AccOwnerWeapon->GetSlotStaticmeshComponents(Slot))
	{
		if (UStaticMesh* DefaultAccMesh = *AccOwnerWeapon->DefaultAccMeshes.Find(Slot))
		{
			AccMesh->SetStaticMesh(DefaultAccMesh);
		}
		else
		{
			if (UWeaponItem* Datas = AccOwnerWeapon->WeaponItemClass.GetDefaultObject())
			{
				if (Datas->DefaultMag != nullptr)
				{
					switch (Acc->AccType)
					{
					default:
						break;
					case EWeaponAccType::E_Mag:
						/**
						 * If the weapon comes with mudular parts, it will have a default mag which will need to be attached if the current mag unequipped
						 */

						//AccMesh->SetStaticMesh(Datas->DefaultMag);
						break;
					}
				}
			}
		}
		//For some acc like muzzle, grip there is no naked mesh
	}
}

void ASurvivalCharacter::Server_EquipAccessories_Implementation(class UItem* ItemBase, bool bFronGround, class AWeapon* Weapon)
{
	EquipAccessories(ItemBase, bFronGround, Weapon);
}

bool ASurvivalCharacter::EquipAccessories(class UItem* ItemBase, bool bFromGround, class AWeapon* Weapon)
{
	if (!HasAuthority()) { Server_EquipAccessories( ItemBase, bFromGround, Weapon); return false; }
	if (IsValid(Weapon))
	{
		if (ItemBase)
		{
			UAccItem* ItemWeaponAcc = Cast<UAccItem>(ItemBase);
			EWeaponAccType AccType = ItemWeaponAcc->AccType;
			UAccItem* ReplacedAccObject = nullptr;

			switch (AccType)
			{
			case EWeaponAccType::E_Muzzle:
			{
				ReplacedAccObject = Weapon->AccMuzzleObject;
			}
				break;
			case EWeaponAccType::E_Mag:
			{
				ReplacedAccObject = Weapon->AccMagObject;
			}
				break;
			default:
				break;
			}
			UpdateWeaponAcc(ItemWeaponAcc, Weapon->Position, ItemWeaponAcc->AccType);
		}
	}
	return false;
}

void ASurvivalCharacter::UpdateWeaponAcc(class UAccItem* ItemWeaponAcc, EWeaponPosition Position, EWeaponAccType AccType)
{
	if (ItemWeaponAcc)
	{
		switch (Position)
		{
		case EWeaponPosition::E_Left:
		{
			if (IsValid(GetPrimaryWeapon()))
			{
				AccOwnerWeapon = GetPrimaryWeapon();
				print("PrimaryGun want to update");
			}
			else
			{
				AccOwnerWeapon = GetHoldWeapon();
				print("HoldGun want to update");
			}
		}
		break;
		case EWeaponPosition::E_Right:
		{
			if (IsValid(GetSecondaryWeapon()))
			{
				AccOwnerWeapon = GetSecondaryWeapon();
				print("SecondaryGun want to update");
			}
			else
			{
				AccOwnerWeapon = GetHoldWeapon();
				print("GetHoldGun want to update");
			}
		}
		break;
		default:
			break;
		}
		if (IsValid(AccOwnerWeapon))
		{
			switch (AccType)
			{
			case EWeaponAccType::E_Muzzle:
			{
				//AccOwnerWeapon->UpdateMuzzle(ItemWeaponAcc);
			}
			break;
			case EWeaponAccType::E_Mag:
			{
				//AccOwnerWeapon->UpdateMag(ItemWeaponAcc);
			}
			break;
			default:
				break;


				bool bCheckValid = 0;
				if (ItemWeaponAcc) {
					bCheckValid = 1;
				}
				else {
					bCheckValid = 0;
				}

				//GetEquippedItems().Add(ItemWeaponAcc->Slot, ItemWeaponAcc);
				//OnEquippedItemsChanged.Broadcast(ItemWeaponAcc->Slot, ItemWeaponAcc);
				OnWeaponAccChanged.Broadcast(AccOwnerWeapon, !bCheckValid, ItemWeaponAcc, AccType);
			}
		}
	}
}
void ASurvivalCharacter::Server_SwitchToSecondaryWeapon_Implementation()
{
	SwitchToSecondaryWeapon();
}

void ASurvivalCharacter::_EquipWeapon()
{
	SetIsHoldWeapon(true);
	SetHoldWeapon(ReadyEquipWeapon);
	ReadyEquipWeapon->bIsOnHand = true;

	switch (ReadyEquipWeapon->Position)
	{
	case EWeaponPosition::E_Left:
	{
		SetPrimaryWeapon(nullptr);
	}
	break;
	case EWeaponPosition::E_Right:
	{
		SetSecondaryWeapon(nullptr);
	}
	break;
	case EWeaponPosition::E_Pan:
		break;
	case EWeaponPosition::E_Grenade:
		break;
	case EWeaponPosition::E_MAX:
		break;
	default:
		break;
	}
	ReadyEquipWeapon = nullptr;
}

void ASurvivalCharacter::Event_OnMontageEnded(class UAnimMontage* Montage, bool Interrupted)
{
	if (!IsValid(GetCurrentMontage()))
	{
		bIsPlayingMontage = false;
	}
}

void ASurvivalCharacter::Event_OnUnEquip()
{
	if (HasAuthority() && IsValid(GetHoldWeapon()))
	{
		SetIsHoldWeapon(false);
		GetHoldWeapon()->bIsOnHand = false;

		switch (GetHoldWeapon()->Position)
		{
		case EWeaponPosition::E_Left:
		{
			if (IsValid(GetHoldWeapon()))
			{
				/**
				* if the weapon position is left side, set the hold weapon to left side slot
				* if the hold weapon is successfully copied to left side slot, set the hold weapon to nullptr
				*/
				SetPrimaryWeapon(GetHoldWeapon());
				if (IsValid(GetPrimaryWeapon()))
				{
					SetHoldWeapon(nullptr);
					print("Left");
				}
				else
				{
					print("Left Side weapon is nullptr :: Character::Event_OnUnEquip()");
					return;
				}
			}
		}
		break;
		case EWeaponPosition::E_Right:
		{
			if (IsValid(GetHoldWeapon()))
			{
				/**
				* if the weapon position is right side, set the hold weapon to right side slot
				* if the hold weapon is successfully copied to right side slot, set the hold weapon to nullptr
				*/
				SetSecondaryWeapon(GetHoldWeapon());
				if (IsValid(GetSecondaryWeapon()))
				{
					SetHoldWeapon(nullptr);
					print("Right");
				}
				else
				{
					print("Right Side weapon is nullptr :: ASurvivalCharacter::Event_OnUnEquip()");
					return;
				}
			}
		}
		break;
		default:
			break;
		}

		//Wen the weapon is switched by pressing 1/2 , it will check if any weapon wants to be equipped
		if (IsValid(ReadyEquipWeapon))
		{
			PlayMontage(EMontageType::EIS_Equip, 0);
		}
		else
		{
			print("Ready Equip weapon is null");
			return;
		}
	}
}

void ASurvivalCharacter::Event_OnEquip()
{
	if (IsValid(ReadyEquipWeapon))
	{
		SetIsHoldWeapon(true);
		SetHoldWeapon(nullptr);
		SetHoldWeapon(ReadyEquipWeapon);

		//Testing
		GetHoldWeapon()->Item = ReadyEquipWeapon->Item;
		OnRep_EquippedWeapon();
		GetHoldWeapon()->OnEquip();
		//Testing finished

		ReadyEquipWeapon->bIsOnHand = true;

		switch (ReadyEquipWeapon->Position)
		{
		case EWeaponPosition::E_Left:
		{
			SetPrimaryWeapon(nullptr);
			ReadyEquipWeapon = nullptr;
		}
		break;
		case EWeaponPosition::E_Right:
		{
			SetSecondaryWeapon(nullptr);
			ReadyEquipWeapon = nullptr;
		}
		break;
		case EWeaponPosition::E_Pan:
			break;
		case EWeaponPosition::E_Grenade:
			break;
		case EWeaponPosition::E_MAX:
			break;
		default:
			break;
		}
	}
}

void ASurvivalCharacter::TakeBackGunKey()
{
	if (HasAuthority())
	{
		Multicast_TakeBackGunKey();
	}
	else
	{
		Server_TakeBackGunKey();
	}
}

void ASurvivalCharacter::Server_TakeBackGunKey_Implementation()
{
	TakeBackGunKey();
}

void ASurvivalCharacter::Multicast_TakeBackGunKey_Implementation()
{
	if (IsValid(GetHoldWeapon()))
	{
		PlayMontage(EMontageType::EIS_UnEquip, 0);
	}
}

void ASurvivalCharacter::UpdateWeapnState()
{
	if (IsValid(GetHoldWeapon()))
	{
		SetIsHoldWeapon(true);
	}
	else
	{
		SetIsHoldWeapon(false);
	}
}

void ASurvivalCharacter::TakeBackWeapon()
{
	if (!IsValid(GetHoldWeapon()))
	{
		print("GetHoldWeapon is null:: @character :: TakeBackWeapon");
		return;
	}
	GetHoldWeapon()->bIsOnHand = false;
	SetIsHoldWeapon(false);

	switch (GetHoldWeapon()->Position)
	{
	case EWeaponPosition::E_Left:
	{
		SetPrimaryWeapon(GetHoldWeapon());
	}
	break;
	case EWeaponPosition::E_Right:
	{
		SetSecondaryWeapon(GetHoldWeapon());
	}
	break;
	default:
		break;
	}

	SetHoldWeapon(nullptr);
}

void ASurvivalCharacter::PlayMontage(EMontageType MontageTypes, float PlayRate)
{

	if (HasAuthority())
	{
		NetMulticast_PlayMontage(MontageTypes, PlayRate);
	}
	else
	{
		Server_PlayMontage(MontageTypes, PlayRate);
	}
}

void ASurvivalCharacter::NetMulticast_PlayMontage_Implementation(EMontageType MontageTypes, float PlayRate)
{
	bIsPlayingMontage = true;

	switch (MontageTypes)
	{
	default:
		break;
	case EMontageType::EIS_Equip:
	{
		if (IsValid(EquipMontage))
		{
			PlayAnimMontage(EquipMontage);
		}
		break;
	case EMontageType::EIS_UnEquip:
	{
		if (IsValid(UnEquipMontage))
		{
			PlayAnimMontage(UnEquipMontage);
		}
	}
	break;
	case EMontageType::EIS_Reload:
		break;
	case EMontageType::EIS_ReloadBullet:
		break;
	case EMontageType::EIS_Fire:
		break;
	case EMontageType::EIS_Use:
		break;
	case EMontageType::EIS_Pickup:
		break;
	case EMontageType::EIS_ThrowGrenade:
		break;
	case EMontageType::EIS_ThrowPetrolBomb:
		break;
	}
	}
}

void ASurvivalCharacter::Server_PlayMontage_Implementation(EMontageType MontageTypes, float PlayRate)
{
	NetMulticast_PlayMontage(MontageTypes, PlayRate);
}

void ASurvivalCharacter::ServerSet_PlayerYaw_Implementation(float Yaw)
{
	PlayerYaw = Yaw;
}

bool ASurvivalCharacter::ServerSet_PlayerYaw_Validate(float Yaw)
{
	return true;
}

void ASurvivalCharacter::ServerSet_PlayerPitch_Implementation(float Pitch)
{
	PlayerPitch = Pitch;
}

bool ASurvivalCharacter::ServerSet_PlayerPitch_Validate(float Pitch)
{
	return true;
}

void ASurvivalCharacter::ServerSetAiming_Implementation(const bool bNewAiming)
{
	SetAiming(bNewAiming);
}

// Called every frame
void ASurvivalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//const bool bIsInteractingOnServer = (GetNetMode() == NM_DedicatedServer && IsInteracting());
	
	if (GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) > InteractionCheckFrequency)
	{
		PerformInteractionCheck();
	}

	if (IsLocallyControlled())
	{
		/*const float DesiredFOV = IsAiming() ? 70.f : 100.f;
		CameraComponent->SetFieldOfView(FMath::FInterpTo(CameraComponent->FieldOfView, DesiredFOV, DeltaTime, 10.f));

		if (EquippedWeapon)
		{
			const FVector ADSLocation = EquippedWeapon->GetWeaponMesh()->GetSocketLocation(NAME_AimDownSightsSocket);
			const FVector DefaultCameraLocation = GetMesh()->GetSocketLocation(FName("CameraSocket"));

			const FVector CameraLoc = bIsAiming ? ADSLocation : DefaultCameraLocation;

			const float InterpSpeed = FVector::Dist(ADSLocation, DefaultCameraLocation) / EquippedWeapon->ADSTime;
			CameraComponent->SetWorldLocation(FMath::VInterpTo(CameraComponent->GetComponentLocation(), CameraLoc, DeltaTime, InterpSpeed));
		}*/
	}

}

void ASurvivalCharacter::Restart()
{
	Super::Restart();

	if (ASurvivalPlayerController* PC = Cast<ASurvivalPlayerController>(GetController()))
	{
		//Show gameplay widget again since it was removed when player died
		if (ASurvivalHUD* HUD = Cast<ASurvivalHUD>(PC->GetHUD()))
		{
			HUD->CreateGameplayWidget();
		}
	}
}

float ASurvivalCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (!IsAlive())
	{
		return 0.f;
	}

	const float DamageDealt = ModifyHealth(-Damage);
	
	UE_LOG(LogTemp, Warning, TEXT("Health: %f"), Health);

	if (Health <= 0.f)
	{
		if (ASurvivalCharacter* KillerCharacter = Cast<ASurvivalCharacter>(DamageCauser->GetOwner()))
		{
			KilledByPlayer(DamageEvent, KillerCharacter, DamageCauser);
		}
		else
		{
			Suicide(DamageEvent, DamageCauser);
		}
	}

	return DamageDealt;
}

void ASurvivalCharacter::SetActorHiddenInGame(bool bNewHidden)
{
	Super::SetActorHiddenInGame(bNewHidden);

	if (EquippedWeapon)
	{
		EquippedWeapon->SetActorHiddenInGame(bNewHidden);
	}
}

void ASurvivalCharacter::SetLootSource(class UInventoryComponent* NewLootSource)
{
	/**If the thing we're looting gets destroyed, we need to tell the client to remove their Loot screen*/
	if (NewLootSource && NewLootSource->GetOwner())
	{
		NewLootSource->GetOwner()->OnDestroyed.AddUniqueDynamic(this, &ASurvivalCharacter::OnLootSourceOwnerDestroyed);
	}

	if (HasAuthority())
	{
		if (NewLootSource)
		{
			//Looting a player keeps their body alive for an extra 2 minutes to provide enough time to loot their items
			if (ASurvivalCharacter* Character = Cast<ASurvivalCharacter>(NewLootSource->GetOwner()))
			{
				Character->SetLifeSpan(120.f);
			}
		}

		LootSource = NewLootSource;
		OnRep_LootSource();
	}
	else
	{
		ServerSetLootSource(NewLootSource);
	}
}

bool ASurvivalCharacter::IsLooting() const
{
	return LootSource != nullptr;
}

void ASurvivalCharacter::BeginLootingPlayer(class ASurvivalCharacter* Character)
{
	if (Character)
	{
		Character->SetLootSource(PlayerInventory);
	}
}

void ASurvivalCharacter::OnLootSourceOwnerDestroyed(AActor* DestroyedActor)
{
	//Remove loot source 
	if (HasAuthority() && LootSource && DestroyedActor == LootSource->GetOwner())
	{
		ServerSetLootSource(nullptr);
	}
}

void ASurvivalCharacter::OnRep_LootSource()
{
	//Bring up or remove the looting menu 
	if (ASurvivalPlayerController* PC = Cast<ASurvivalPlayerController>(GetController()))
	{
		if (PC->IsLocalController())
		{
			if (ASurvivalHUD* HUD = Cast<ASurvivalHUD>(PC->GetHUD()))
			{
				if(LootSource)
				{
					HUD->OpenLootWidget();
				}
				else
				{
					HUD->CloseLootWidget();
				}
			}
		}
	}
}

void ASurvivalCharacter::ServerSetLootSource_Implementation(class UInventoryComponent* NewLootSource)
{
	SetLootSource(NewLootSource);
}

bool ASurvivalCharacter::ServerSetLootSource_Validate(class UInventoryComponent* NewLootSource)
{
	return true;
}

void ASurvivalCharacter::LootItem(class UItem* ItemToGive)
{
	if (HasAuthority())
	{
		if (PlayerInventory && LootSource && ItemToGive && LootSource->HasItem(ItemToGive->GetClass(), ItemToGive->GetQuantity()))
		{
			const FItemAddResult AddResult = PlayerInventory->TryAddItem(ItemToGive);

			if (AddResult.AmountGiven > 0)
			{
				LootSource->ConsumeItem(ItemToGive, AddResult.AmountGiven);
			}
			else
			{
				//Tell player why they couldn't loot the item
				if (ASurvivalPlayerController* PC = Cast<ASurvivalPlayerController>(GetController()))
				{
					PC->ClientShowNotification(AddResult.ErrorText);
				}
			}
		}

		if (PlayerInventory && ItemToGive && ItemToGive->GetClass(), ItemToGive->GetQuantity())
		{
			const FItemAddResult AddResult = PlayerInventory->TryAddItem(ItemToGive);

			if (AddResult.AmountGiven > 0)
			{
				PlayerInventory->ConsumeItem(ItemToGive, AddResult.AmountGiven);
			}
			else
			{
				//Tell player why they couldn't loot the item
				if (ASurvivalPlayerController* PC = Cast<ASurvivalPlayerController>(GetController()))
				{
					PC->ClientShowNotification(AddResult.ErrorText);
				}
			}
		}
	}
	else
	{
		ServerLootItem(ItemToGive);
	}
}

void ASurvivalCharacter::ServerLootItem_Implementation(class UItem* ItemToLoot)
{
	LootItem(ItemToLoot);
}

bool ASurvivalCharacter::ServerLootItem_Validate(class UItem* ItemToLoot)
{
	return true;
}

void ASurvivalCharacter::PerformInteractionCheck()
{

	if (GetController() == nullptr)
	{
		return;
	}
	
	InteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();

	FVector EyesLoc;
	FRotator EyesRot;

	GetController()->GetPlayerViewPoint(EyesLoc, EyesRot);

	FVector TraceStart = EyesLoc;
	FVector TraceEnd = (EyesRot.Vector() * InteractionCheckDistance) + TraceStart;
	FHitResult TraceHit;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(TraceHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		//Check if we hit an interactable object
		//Check if we hit an interactable object
		if (const AActor* HitActor = TraceHit.GetActor())
		{
			if (UInteractionComponent* InteractionComponent = Cast<UInteractionComponent>(HitActor->GetComponentByClass(UInteractionComponent::StaticClass())))
			{
				float Distance = (TraceStart - TraceHit.ImpactPoint).Size();
				if (InteractionComponent != GetInteractable() && Distance <= InteractionComponent->InteractionDistance)
				{
					FoundNewInteractable(InteractionComponent);
				}
				else if (Distance > InteractionComponent->InteractionDistance && GetInteractable())
				{
					CouldntFindInteractable();
				}

				return;
			}
		}
	}

	CouldntFindInteractable();

}

void ASurvivalCharacter::CouldntFindInteractable()
{
	//We've lost focus on an interactable. Clear the timer.
	if (GetWorldTimerManager().IsTimerActive(TimerHandle_Interact))
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_Interact);
	}

	//Tell the interactable we've stopped focusing on it, and clear the current interactable
	if (UInteractionComponent* Interactable = GetInteractable())
	{
		Interactable->EndFocus(this);

		if (InteractionData.bInteractHeld)
		{
			EndInteract();
		}
	}

	InteractionData.ViewedInteractionComponent = nullptr;
}

void ASurvivalCharacter::FoundNewInteractable(UInteractionComponent* Interactable)
{
	EndInteract();

	if (UInteractionComponent* OldInteractable = GetInteractable())
	{
		OldInteractable->EndFocus(this);
	}

	InteractionData.ViewedInteractionComponent = Interactable;
	Interactable->BeginFocus(this);

}

void ASurvivalCharacter::BeginInteract()
{
	if (!HasAuthority())
	{
		ServerBeginInteract();
	}

	/**As an optimization, the server only checks that we're looking at an item once we begin interacting with it.
	This saves the server doing a check every tick for an interactable Item. The exception is a non-instant interact.
	In this case, the server will check every tick for the duration of the interact*/
	if (HasAuthority())
	{
		PerformInteractionCheck();
	}

	InteractionData.bInteractHeld = true;

	if (UInteractionComponent* Interactable = GetInteractable())
	{
		Interactable->BeginInteract(this);

		if (FMath::IsNearlyZero(Interactable->InteractionTime))
		{
			Interact();
		}
		else
		{
			GetWorldTimerManager().SetTimer(TimerHandle_Interact, this, &ASurvivalCharacter::Interact, Interactable->InteractionTime, false);
		}
	}
}

void ASurvivalCharacter::EndInteract()
{
	if (!HasAuthority())
	{
		ServerEndInteract();
	}

	InteractionData.bInteractHeld = false;

	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);

	if (UInteractionComponent* Interactable = GetInteractable())
	{
		Interactable->EndInteract(this);
	}
}

void ASurvivalCharacter::Interact()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);

	if (UInteractionComponent* Interactable = GetInteractable())
	{
		Interactable->Interact(this);
	}
}

void ASurvivalCharacter::ServerEndInteract_Implementation()
{
	EndInteract();
}

bool ASurvivalCharacter::ServerEndInteract_Validate()
{
	return true;
}

void ASurvivalCharacter::ServerBeginInteract_Implementation()
{
	BeginInteract();
}

bool ASurvivalCharacter::ServerBeginInteract_Validate()
{
	return true;
}

void ASurvivalCharacter::StartReload()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->StartReload();
	}
}

// Called to bind functionality to input
void ASurvivalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASurvivalCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASurvivalCharacter::StopFire);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ASurvivalCharacter::StartAiming);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ASurvivalCharacter::StopAiming);
	PlayerInputComponent->BindAction("Throw", IE_Pressed, this, &ASurvivalCharacter::UseThrowable);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ASurvivalCharacter::BeginInteract);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &ASurvivalCharacter::EndInteract);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASurvivalCharacter::StartCrouching);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASurvivalCharacter::StopCrouching);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASurvivalCharacter::StartSprinting);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASurvivalCharacter::StopSprinting);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASurvivalCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASurvivalCharacter::MoveRight);

	PlayerInputComponent->BindAction("TakeBackGun", IE_Pressed, this, &ASurvivalCharacter::TakeBackGunKey);
	PlayerInputComponent->BindAction("KeyboardOneKey", IE_Pressed, this, &ASurvivalCharacter::SwitchToPrimaryWeapon);
	PlayerInputComponent->BindAction("KeyboardTwoKey", IE_Pressed, this, &ASurvivalCharacter::SwitchToSecondaryWeapon);
}

void ASurvivalCharacter::TryPickupItem(APickup* ItemToPickup)
{
	if (ItemToPickup != nullptr)
	{
		ServerPickupItem(ItemToPickup);
	}
}

void ASurvivalCharacter::ExecuteMove(float AxisValue, bool bForward)
{
	if (ASurvivalPlayerController* PC = Cast<ASurvivalPlayerController>(GetController()))
	{
		if (!(GetProne() && GetAiming()))
		{
			if (!(GetPlayingMontage() && GetProne()))
			{
				if (bForward)
				{
					if (GetMoveForwardAxis() != AxisValue && !GetCharacterMovement()->IsFalling())
					{
						SetMoveForwardAxis(AxisValue);// MoveForwardAxis = Axis;
						//CharacterRef->AddMovementInput(CharacterRef->GetActorForwardVector(), AxisValue);

						//CharacterRef->SetWalkSpeed(AxisValue);
						PC->UpdateCameraHeight();
						//						UpdateWeaponDisplay(PC->CalculateHoldGunSocket());
					}
				}
				else if (!bForward)
				{
					if (GetMoveRightAxis() != AxisValue)
					{
						SetMoveRightAxis(AxisValue);// MoveRightAxis = Axis;
						//CharacterRef->AddMovementInput(CharacterRef->GetActorRightVector(), AxisValue);

						//CharacterRef->SetWalkSpeed(AxisValue);
						PC->UpdateCameraHeight();
						//						UpdateWeaponDisplay(PC->CalculateHoldGunSocket());
					}
				}
			}
			if (GetEnableMove())
			{
				UpdateWalkSpeed();
				//MovingOnTheGround(bForward, AxisValue, PC->GetControllerRotation());//GetControlRotation changed to custom GetControllerRotation
				MovingOnTheGround(bForward, AxisValue, PC->GetControllerRotation());//GetControlRotation changed to custom GetControllerRotation
				//print("Update Weapon Display : needs to be called... Player Character : Line 127");
			}
		}
	}
}

void ASurvivalCharacter::MovingOnTheGround(bool bForward, float AxisValue, FRotator ControllerRotation)
{
	//const FRotator Rotation = ControllerRotation;

	//SmoothIncrease
	const float VelocitySize = GetCharacterMovement()->Velocity.Size() + 15.0f;
	if (VelocitySize <= GetWalkSpeed())
	{
		GetCharacterMovement()->MaxWalkSpeed = VelocitySize;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = GetWalkSpeed();
	}

	if (bForward)
	{
		AddMovementInput(GetActorForwardVector(), AxisValue);
	}
	else
	{
		if (ASurvivalPlayerController* PC = Cast<ASurvivalPlayerController>(GetController()))
		{
			// find out which way is right
			//ControllerRotation = Controller->GetControlRotation();
			ControllerRotation = PC->GetControlRotation();
			const FRotator YawRotationa(0, ControllerRotation.Yaw, 0);
			// get right vector 

			FVector Direction = FRotationMatrix(YawRotationa).GetUnitAxis(EAxis::Y);
			AddMovementInput(Direction, AxisValue);
		}
	}


}

void ASurvivalCharacter::UpdateWalkSpeed()
{
	if (const UDataTable * DT_WalkSpeed{ LoadObject<UDataTable>(GetWorld(), TEXT("/Game/Datas/Character/Speed/DT_WalkSpeed")) })
	{
		FString HoldWeapon;
		FString Posture;
		FString MoveState;
		/*Get the current character state status*/
		HoldWeapon = UKismetStringLibrary::Conv_BoolToString(GetIsHoldWeapon());

		if (GetCrouching())
		{
			Posture = FString(TEXT("crouch"));
		}
		else if (GetProne())
		{
			Posture = FString(TEXT("prone"));
		}
		else
		{
			Posture = FString(TEXT("stand"));
		}

		if (GetAiming())
		{
			MoveState = FString(TEXT("aim"));
		}
		else if (GetWalkPressed())
		{

			MoveState = FString(TEXT("walk"));
		}
		else if (GetRunPressed())
		{
			MoveState = FString(TEXT("run"));
		}
		else
		{
			MoveState = FString(TEXT("jog"));
		}
		/*Get Speed Values through Character Stats*/
		//Appending Strings
		FString Str = HoldWeapon;
		Str.Append("_");
		Str.Append(Posture);
		Str.Append("_");
		Str.Append(MoveState);
		//FString value(FName(Str));
		if (const FST_WalkSpeed * RowDatas{ DT_WalkSpeed->FindRow<FST_WalkSpeed>(FName(*Str), "") })
		{
			if (GetMoveForwardAxis() > 0.0f)
			{
				SetWalkSpeed(RowDatas->Forward);
			}
			else
			{
				SetWalkSpeed(RowDatas->Others);
			}
		}
	}
}

void ASurvivalCharacter::AppendCharacterState(bool bWithGun, FString Current, FString Target)
{
	if (const UDataTable * DT_MoveTimeDelay{ LoadObject<UDataTable>(GetWorld(), TEXT("/Game/Datas/Character/DT_MoveTimeDelay")) })
	{
		FTimerHandle DelayHandle;
		FString CurrentState{};
		CurrentState = UKismetStringLibrary::Conv_BoolToString(bWithGun);

		FString Str = CurrentState;
		Str.Append("_");
		Str.Append(Current);
		Str.Append("_");
		Str.Append(Target);

		if (const FST_MoveTimeDelay * RowDatas{ DT_MoveTimeDelay->FindRow<FST_MoveTimeDelay>(FName(*Str), "") })
		{
			SetEnableMove(false);
			GetWorld()->GetTimerManager().SetTimer(DelayHandle, this, &ASurvivalCharacter::HelperEnableDisableMove, RowDatas->Sec, false);
		}
		else
		{
			printText("Current State is not valid, Movement can't be delayed");
		}
	}
	else
	{
		printText("Data Table Not valid :: DT_MoveTimeDelay ");
	}
}

void ASurvivalCharacter::HelperEnableDisableMove()
{
	SetEnableMove(true);
}

void ASurvivalCharacter::ServerPickupItem_Implementation(APickup* ItemToPickup)
{
	ItemToPickup->OnTakePickup(this);
}

#undef LOCTEXT_NAMESPACE