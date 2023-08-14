// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SurvivalCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PawnMovementComponent.h"
#include "SurvivalAnimInstance.generated.h"


//-------------------<<<---------->>>
//DELEGATES
//-------------------<<<---------->>>

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquip);
UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipEnd);
UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUnEquip);
UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUnEquipEnd);
UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReloadEnd);
UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFireEnd);

/**
 * 
 */

UCLASS()
class SURVIVALGAME_API USurvivalAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	//Default constructor
	USurvivalAnimInstance();

	//In case when Equip Started
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, BlueprintAssignable, BlueprintCallable,Category = "Default")
	FOnEquip OnEquip;
	//In case when Equip Ends
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, BlueprintAssignable, BlueprintCallable, Category = "Default")
	FOnEquipEnd OnEquipEnd;
	//In case when Un Equip Started
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, BlueprintAssignable, BlueprintCallable, Category = "Default")
	FOnUnEquip OnUnEquip;
	//In case when Un Equip Ends
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, BlueprintAssignable, BlueprintCallable, Category = "Default")
	FOnUnEquipEnd OnUnEquipEnd;
	//In case when Reload Ends
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, BlueprintAssignable, BlueprintCallable, Category = "Default")
	FOnReloadEnd OnReloadEnd;
	//In case when Firing Ends
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, BlueprintAssignable, BlueprintCallable, Category = "Default")
	FOnFireEnd OnFireEnd;

protected:
	//BeginPlay of the animation, runs only at start
	virtual void NativeInitializeAnimation() override;
	//Event Update animation
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	//Try Get Pawn Owner, this is a pawn and character refference
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Refferences | Pawn")
		ASurvivalCharacter* Character;

	//GamePlay variables
	//Defining speed of the character
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
		float Speed;
	
	//Defining ForwardValue of the character
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
		float ForwardValue;

	//Defining direction of the character
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
		float Direction;
	
	//Defining Yaw of the character
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
		float Yaw;

	//Defining Pitch of the character
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
		float Pitch;

	
	//Defining Sideways velocity of the character
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
		float SidewaysVelocity;

	//Defining AimOffset of the character to be played by inter get index
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
		int32 AimOffsetType;
	
	//Defining can character Jump
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
		bool bJumping_Anim;
	
	//Defining can character crouch
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
		bool bCrouching_Anim;
	
	//Defining can character prone
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Default")
		bool bProning_Anim;
	
	//Defining can character aim
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,  Category = "Default")
		bool bAiming_Anim;
	
	//Defining can character holding aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,  Category = "Default")
		bool bHoldAiming_Anim;
	
	//Defining can character Equip
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,  Category = "Default")
		bool bEquip_Anim;
	
	//Defining can character Equip
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,  Category = "Default")
		bool bSightAiming_Anim;
	
	//Defining can character firing
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,  Category = "Default")
		bool bFiring_Anim;
	
	//Defining can character reload
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,  Category = "Default")
		bool bReload_Anim;
	
	//Defining can character aim accurate
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,  Category = "Default")
		bool bAimAccurate_Anim;
	
	//Defining can character holding weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,  Category = "Default")
		bool bHoldWeapon_Anim;
	
	//Defining can character in air
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,  Category = "Default")
		bool bInAir_Anim;

	//Defining can lean to right
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,  Category = "Default")
		bool bOnTheAir_Anim;
	
	//Defining can character freefall
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,  Category = "Default")
		bool bFreefalling_Anim;
	
	//Defining can character open umbrella
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,  Category = "Default")
		bool bUmbrellaOpen_Anim;
	
	//Defining can character dead
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,  Category = "Default")
		bool bDead_Anim;
	
	//Defining can montage be played
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,  Category = "Default")
		bool bPlayingMontage_Anim;
	
	//Defining can lean to right
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,  Category = "Default")
		bool bAltPressed_Anim;
	
	//Defining can lean to right
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,  Category = "Default")
		bool bWalkPressed_Anim;
	
	//Defining can lean to right
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,  Category = "Default")
		bool bRunPressed_Anim;
	
	//Defining can lean to right
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,  Category = "Default")
		bool bFreeFalling_Anims;
	
	//Defining can lean to right
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,  Category = "Default")
		bool bEnableMove_Anim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,  Category = "Default")
		class AWeapon* Weapon;

	//Used to update the animation properties
	UFUNCTION(BlueprintCallable, Category = "Animations|Update")
	virtual void UpdateAnimProperties();


};
