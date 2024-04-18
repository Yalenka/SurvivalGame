// Fill out your copyright notice in the Description page of Project Settings.


#include "SurvivalAnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/Rotator.h"
#include "GameFramework/PlayerController.h"
#include "../Player/SurvivalPlayerController.h"
#include "../Utilities/Loggers/DebugUtilityLogger.h"


USurvivalAnimInstance::USurvivalAnimInstance()
{

}

void USurvivalAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Character = Cast<ASurvivalCharacter>(TryGetPawnOwner());
}

void USurvivalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (Character/* && TryGetPawnOwnerRef->IsLocallyControlled()*/)
	{
		UpdateAnimProperties();
	}
	else
	{
		/*TArray<AActor*>OutActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASurvivalCharacter::StaticClass(), OutActors);

		for (AActor* Actor: OutActors)
		{
			TryGetPawnOwnerRef = Cast<ASurvivalCharacter>(Actor);
			if (TryGetPawnOwnerRef)
			{
				UpdateAnimProperties();
				break;;;
			}
		}*/
	}
}

void USurvivalAnimInstance::UpdateAnimProperties()
{
	bCrouching_Anim = Character->GetCrouching();
	bProning_Anim = Character->GetProne();
	bDead_Anim = Character->GetDead();
	Weapon = Character->GetHoldWeapon();

	if (Weapon != nullptr)
	{
		bHoldAiming_Anim = true;
	}
	else
	{
		bHoldWeapon_Anim = false;
	}

	bAiming_Anim = Character->GetAiming();
	//bHoldWeapon_Anim = TryGetPawnOwnerRef->GetHoldWeapon();

	bAltPressed_Anim = Character->GetAltPressed();
	bWalkPressed_Anim = Character->GetWalkPressed();
	bRunPressed_Anim = Character->GetRunPressed();
	bPlayingMontage_Anim = Character->GetPlayingMontage();
	bEnableMove_Anim = Character->GetEnableMove();

	//bOnTheAir_Anim = TryGetPawnOwnerRef->GetMovementComponent()->IsFalling();
	bInAir_Anim = Character->GetMovementComponent()->IsFalling();

	ForwardValue = Character->GetMoveForwardAxis();

	/*Set speed value from anim bp on character and detect if falling then do a jump*/
	//Set 'IsInAir' (used in state machine)
	FVector BreakUnRotator = Character->GetActorRotation().UnrotateVector(Character->GetVelocity());
	SidewaysVelocity = BreakUnRotator.Y;
	
	//Setting 'Speed' (use in 1D blend space)
	Speed = Character->GetVelocity().Size();

	//Set Direction Variable from anim bp on character
	Direction = CalculateDirection(Character->GetVelocity(), Character->GetActorRotation());
	
	//Set Pitch and Yaw Variable
	//FRotator BreakDeltaRotator = TryGetPawnOwnerRef->GetControlRotation() - TryGetPawnOwnerRef->GetActorRotation();
	//BreakDeltaRotator.Normalize();
	//Pitch = BreakDeltaRotator.Pitch;
	//Yaw = BreakDeltaRotator.Yaw;

	/**Getting/Setting replicated normalized values from character class*/
	Pitch = Character->PlayerPitch;
	Yaw = Character->PlayerYaw;

	//Set AimOffset Types Values
	if (bProning_Anim | bInAir_Anim)
	{
		AimOffsetType = 0;
	}
	//Crouch States with hold/un-hold weapon, aim/un-aim and and aim state
	else if (bCrouching_Anim)
	{
		//Crouch and holding weapon
		if (bHoldWeapon_Anim)
		{
			//Crouch, holding weapon and aim
			if (bAiming_Anim)
			{
				//armed aim crouch
				AimOffsetType = 6;


			}
			//Crouch, holding weapon and !aim
			else
			{
				//armed crouch, not aimed
				AimOffsetType = 4;
			}
		}

		else
		{
			//unarmed crouch
			AimOffsetType = 2;
		}
	}
	else
	{
		if (bHoldWeapon_Anim)
		{
			//Aim State Stand
			if (bAiming_Anim)
			{
				//armed aim stand
				AimOffsetType = 5;
			}
			else
			{
				//armed stand
				AimOffsetType = 3;
			}
		}
		else
		{
			//unarmed stand
			AimOffsetType = 1;
		}
	}


	//float BaseAimRotation = TryGetPawnOwnerRef->GetBaseAimRotation().Pitch; 
	//float AimOffsetPitch = FMath::ClampAngle(BaseAimRotation, 0.0f, 100.0f);
}
