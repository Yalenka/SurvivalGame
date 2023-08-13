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
	TryGetPawnOwnerRef = Cast<ASurvivalCharacter>(TryGetPawnOwner());
}

void USurvivalAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (TryGetPawnOwnerRef/* && TryGetPawnOwnerRef->IsLocallyControlled()*/)
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
	bCrouching_Anim = TryGetPawnOwnerRef->GetCrouching();
	bProning_Anim = TryGetPawnOwnerRef->GetProne();
	bDead_Anim = TryGetPawnOwnerRef->GetDead();
	Weapon = TryGetPawnOwnerRef->GetEquippedWeapon();

	if (Weapon != nullptr)
	{
		bHoldAiming_Anim = true;
	}
	else
	{
		bHoldWeapon_Anim = false;
	}

	bAiming_Anim = TryGetPawnOwnerRef->GetAiming();
	//bHoldWeapon_Anim = TryGetPawnOwnerRef->GetHoldWeapon();

	bAltPressed_Anim = TryGetPawnOwnerRef->GetAltPressed();
	bWalkPressed_Anim = TryGetPawnOwnerRef->GetWalkPressed();
	bRunPressed_Anim = TryGetPawnOwnerRef->GetRunPressed();
	bPlayingMontage_Anim = TryGetPawnOwnerRef->GetPlayingMontage();
	bEnableMove_Anim = TryGetPawnOwnerRef->GetEnableMove();

	//bOnTheAir_Anim = TryGetPawnOwnerRef->GetMovementComponent()->IsFalling();
	bInAir_Anim = TryGetPawnOwnerRef->GetMovementComponent()->IsFalling();

	ForwardValue = TryGetPawnOwnerRef->GetMoveForwardAxis();

	/*Set speed value from anim bp on character and detect if falling then do a jump*/
	//Set 'IsInAir' (used in state machine)
	FVector BreakUnRotator = TryGetPawnOwnerRef->GetActorRotation().UnrotateVector(TryGetPawnOwnerRef->GetVelocity());
	SidewaysVelocity = BreakUnRotator.Y;
	
	//Setting 'Speed' (use in 1D blend space)
	Speed = TryGetPawnOwnerRef->GetVelocity().Size();

	//Set Direction Variable from anim bp on character
	Direction = CalculateDirection(TryGetPawnOwnerRef->GetVelocity(), TryGetPawnOwnerRef->GetActorRotation());
	
	//Set Pitch and Yaw Variable
	FRotator BreakDeltaRotator = TryGetPawnOwnerRef->GetControlRotation() - TryGetPawnOwnerRef->GetActorRotation();
	BreakDeltaRotator.Normalize();
	Pitch = BreakDeltaRotator.Pitch;
	Yaw = BreakDeltaRotator.Yaw;

	printf("Turn Rate: %f", Yaw);

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
