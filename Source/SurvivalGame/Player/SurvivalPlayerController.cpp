//Extended verison 2023 By [Yalenka] - SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020


#include "SurvivalPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SurvivalCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Widgets/SurvivalHUD.h"
#include "Framework/SurvivalGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"

#define LOCTEXT_NAMESPACE "SurvivalPlayerController"

ASurvivalPlayerController::ASurvivalPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	ConstructTimeLinesAndBind();
}

void ASurvivalPlayerController::ClientShowNotification_Implementation(const FText& Message)
{
	if (ASurvivalHUD* HUD = Cast<ASurvivalHUD>(GetHUD()))
	{
		HUD->ShowNotification(Message);
	}
}

void ASurvivalPlayerController::Died(class ASurvivalCharacter* Killer)
{
	if (ASurvivalGameStateBase* GS = Cast<ASurvivalGameStateBase>(UGameplayStatics::GetGameState(GetWorld())))
	{
		//Force the player to respawn 
		FTimerHandle DummyHandle;
		GetWorldTimerManager().SetTimer(DummyHandle, this, &ASurvivalPlayerController::Respawn, GS->RespawnTime, false);

		if (ASurvivalHUD* HUD = Cast<ASurvivalHUD>(GetHUD()))
		{
			HUD->ShowDeathWidget(Killer);
		}
	}
}

void ASurvivalPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("Turn", this, &ASurvivalPlayerController::Turn);
	InputComponent->BindAxis("LookUp", this, &ASurvivalPlayerController::LookUp);

	InputComponent->BindAction("OpenInventory", IE_Pressed, this, &ASurvivalPlayerController::OpenInventory);

	InputComponent->BindAction("Pause", IE_Pressed, this, &ASurvivalPlayerController::PauseGame);

	InputComponent->BindAction("OpenMap", IE_Pressed, this, &ASurvivalPlayerController::OpenMap);
	InputComponent->BindAction("OpenMap", IE_Released, this, &ASurvivalPlayerController::CloseMap);

	InputComponent->BindAction("Reload", IE_Pressed, this, &ASurvivalPlayerController::StartReload);

	InputComponent->BindAction("FreeLook", IE_Pressed, this, &ASurvivalPlayerController::StartFreeLook);
	InputComponent->BindAction("FreeLook", IE_Released, this, &ASurvivalPlayerController::StopFreeLook);

}

void ASurvivalPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	ptr_Character = Cast<ASurvivalCharacter>(GetPawn());
	if (!ptr_Character)
	{
		print("ASurvivalCharacter cast failed #ASurvivalPlayerController::BeginPlay()");
		return;
	}
}

void ASurvivalPlayerController::ClientShotHitConfirmed_Implementation()
{
	if (ASurvivalHUD* HUD = Cast<ASurvivalHUD>(GetHUD()))
	{
		HUD->ShowHitmarker();
	}

}

void ASurvivalPlayerController::Respawn()
{
	UnPossess();
	ChangeState(NAME_Inactive);

	if (!HasAuthority())
	{
		ServerRespawn();
	}
	else
	{
		ServerRestartPlayer();
	}
}

bool ASurvivalPlayerController::IsInventoryOpen() const
{
	if (ASurvivalHUD* HUD = Cast<ASurvivalHUD>(GetHUD()))
	{
		return HUD->IsInventoryOpen();
	}

	return false;
}

void ASurvivalPlayerController::ServerRespawn_Implementation()
{
	Respawn();
}

bool ASurvivalPlayerController::ServerRespawn_Validate()
{
	return true;
}

void ASurvivalPlayerController::OpenInventory()
{
	if (ASurvivalHUD* HUD = Cast<ASurvivalHUD>(GetHUD()))
	{
		HUD->OpenInventoryWidget();
	}
}

void ASurvivalPlayerController::OpenMap()
{
	if (ASurvivalHUD* HUD = Cast<ASurvivalHUD>(GetHUD()))
	{
		HUD->OpenMap();
	}
}

void ASurvivalPlayerController::CloseMap()
{
	if (ASurvivalHUD* HUD = Cast<ASurvivalHUD>(GetHUD()))
	{
		HUD->CloseMap();
	}
}

void ASurvivalPlayerController::PauseGame()
{
	if (ASurvivalHUD* HUD = Cast<ASurvivalHUD>(GetHUD()))
	{
		HUD->OpenPauseWidget();
	}
}

void ASurvivalPlayerController::ResumeGame()
{
	if (ASurvivalHUD* HUD = Cast<ASurvivalHUD>(GetHUD()))
	{
		HUD->ClosePauseWidget();
	}
}

void ASurvivalPlayerController::ApplyRecoil(const FVector2D& RecoilAmount, const float RecoilSpeed, const float RecoilResetSpeed)
{
	if (IsLocalPlayerController())
	{
		RecoilBumpAmount += RecoilAmount;
		RecoilResetAmount += -RecoilAmount;

		CurrentRecoilSpeed = RecoilSpeed;
		CurrentRecoilResetSpeed = RecoilResetSpeed;

		LastRecoilTime = GetWorld()->GetTimeSeconds();
	}
}

void ASurvivalPlayerController::Turn(float Rate)
{
	//If the player has moved their camera to compensate for recoil we need this to cancel out the recoil reset effect
	if (!FMath::IsNearlyZero(RecoilResetAmount.X, 0.01f))
	{
		if (RecoilResetAmount.X > 0.f && Rate > 0.f)
		{
			RecoilResetAmount.X = FMath::Max(0.f, RecoilResetAmount.X - Rate);
		}
		else if (RecoilResetAmount.X < 0.f && Rate < 0.f)
		{
			RecoilResetAmount.X = FMath::Min(0.f, RecoilResetAmount.X - Rate);
		}
	}

	//Apply the recoil over several frames
	if (!FMath::IsNearlyZero(RecoilBumpAmount.X, 0.1f))
	{
		FVector2D LastCurrentRecoil = RecoilBumpAmount;
		RecoilBumpAmount.X = FMath::FInterpTo(RecoilBumpAmount.X, 0.f, GetWorld()->DeltaTimeSeconds, CurrentRecoilSpeed);

		AddYawInput(LastCurrentRecoil.X - RecoilBumpAmount.X);
	}

	//Slowly reset back to center after recoil is processed
	FVector2D LastRecoilResetAmount = RecoilResetAmount;
	RecoilResetAmount.X = FMath::FInterpTo(RecoilResetAmount.X, 0.f, GetWorld()->DeltaTimeSeconds, CurrentRecoilResetSpeed);
	AddYawInput(LastRecoilResetAmount.X - RecoilResetAmount.X);

	if (ptr_Character)
	{
		ptr_Character->Turn(Rate);
	}
}

void ASurvivalPlayerController::LookUp(float Rate)
{
	if (!FMath::IsNearlyZero(RecoilResetAmount.Y, 0.01f))
	{
		if (RecoilResetAmount.Y > 0.f && Rate > 0.f)
		{
			RecoilResetAmount.Y = FMath::Max(0.f, RecoilResetAmount.Y - Rate);
		}
		else if (RecoilResetAmount.Y < 0.f && Rate < 0.f)
		{
			RecoilResetAmount.Y = FMath::Min(0.f, RecoilResetAmount.Y - Rate);
		}
	}

	//Apply the recoil over several frames
	if (!FMath::IsNearlyZero(RecoilBumpAmount.Y, 0.01f))
	{
		FVector2D LastCurrentRecoil = RecoilBumpAmount;
		RecoilBumpAmount.Y = FMath::FInterpTo(RecoilBumpAmount.Y, 0.f, GetWorld()->DeltaTimeSeconds, CurrentRecoilSpeed);

		AddPitchInput(LastCurrentRecoil.Y - RecoilBumpAmount.Y);
	}

	//Slowly reset back to center after recoil is processed
	FVector2D LastRecoilResetAmount = RecoilResetAmount;
	RecoilResetAmount.Y = FMath::FInterpTo(RecoilResetAmount.Y, 0.f, GetWorld()->DeltaTimeSeconds, CurrentRecoilResetSpeed);
	AddPitchInput(LastRecoilResetAmount.Y - RecoilResetAmount.Y);

	if (ptr_Character)
	{
		ptr_Character->LookUp(Rate);
	}
}

void ASurvivalPlayerController::StartReload()
{
	if (ASurvivalCharacter* SurvivalCharacter = Cast<ASurvivalCharacter>(GetPawn()))
	{
		if (SurvivalCharacter->IsAlive())
		{
			SurvivalCharacter->StartReload();
		}
		else // R key should respawn the player if dead
		{
			Respawn();
		}
	}
}

void ASurvivalPlayerController::ConstructTimeLinesAndBind()
{
	//Camera Height Update Timeline
	static ConstructorHelpers::FObjectFinder<UCurveFloat> fCurveCH(TEXT("/Game/Blueprints/TL_Curves/C_FC_TLUpdateCameraHeight"));
	if (fCurveCH.Object)
	{
		CamHeightFloatCurve = fCurveCH.Object;
		//Timeline components don't need to be attached/connect to anything.
		TL_CamHeightComponent = CreateDefaultSubobject<UTimelineComponent>(TEXT("TL_UpdateCameraHeight"));
		//Binding Delegates with UFunctions
		On_CameraHeightUpdate.BindUFunction(this, FName("TL_CamHeight_Update"));
		//Add the float curve named "Percent" to the timeline and connect to the interpfunction's delegate
		TL_CamHeightComponent->AddInterpFloat(CamHeightFloatCurve, On_CameraHeightUpdate, FName("Percent"));
		//Setting Timeline Settings before we start it
		TL_CamHeightComponent->SetLooping(false);
		TL_CamHeightComponent->SetIgnoreTimeDilation(false);
		TL_CamHeightComponent->SetTimelineLength(0.70f);
	}
	//Camera Restore Update Timeline
	static ConstructorHelpers::FObjectFinder<UCurveFloat> TryLoadFreeLookCurve(TEXT("/Game/Blueprints/TL_Curves/C_FC_TLSpringArmRestore"));
	if (TryLoadFreeLookCurve.Object)
	{
		FreeLookFloatCurve = TryLoadFreeLookCurve.Object;
		//Timeline components don't need to be attached/connect to anything.
		TL_FreelookComponent = CreateDefaultSubobject<UTimelineComponent>(TEXT("TL_RestoreCamera"));
		//Binding Delegates with UFunctions
		On_FreelookUpdate.BindUFunction(this, FName("TL_FreeLook_Update"));
		On_FreelookFinished.BindUFunction(this, FName("TL_FreeLook_Finished"));
		//Add the float curve named "Percentage" to the timeline and connect to the interpfunction's delegate
		TL_FreelookComponent->AddInterpFloat(FreeLookFloatCurve, On_FreelookUpdate, FName("Percentage"));
		TL_FreelookComponent->SetTimelineFinishedFunc(On_FreelookFinished);
		//Setting Timeline Properties before we start it
		TL_FreelookComponent->SetLooping(false);
		TL_FreelookComponent->SetIgnoreTimeDilation(true);
		TL_FreelookComponent->SetTimelineLength(1.70f);
	}
}

void ASurvivalPlayerController::UpdateCameraHeight()
{
	if (ASurvivalCharacter* CharacterRef = Cast<ASurvivalCharacter>(GetPawn()))
	{
		if (const UDataTable * DT_CameraHeight{ LoadObject<UDataTable>(GetWorld(), TEXT("/Game/Datas/Character/Camera/DT_CameraHeight")) })
		{
			if (CharacterRef->GetCrouching())
			{
				if (!CharacterRef->GetIsHoldWeapon() && (CharacterRef->GetMoveForwardAxis() != 0.0f || CharacterRef->GetMoveRightAxis() != 0.0f))
				{
					if (const FST_CameraHeight * Row{ DT_CameraHeight->FindRow<FST_CameraHeight>("Crouch_Normal_Move", "") }) //60
					{
						NewHeight = Row->Height;
					}
				}
				else if (CharacterRef->GetIsHoldWeapon() && (CharacterRef->GetMoveForwardAxis() != 0.0f || CharacterRef->GetMoveRightAxis() != 0.0f))
				{
					if (const FST_CameraHeight * Row{ DT_CameraHeight->FindRow<FST_CameraHeight>("Crouch_Rifle_Move", "") }) //80
					{
						NewHeight = Row->Height;
					}
				}
				else
				{
					if (const FST_CameraHeight * Row{ DT_CameraHeight->FindRow<FST_CameraHeight>("Crouch", "") }) //40
					{
						NewHeight = Row->Height;
					}
				}
			}
			else if (CharacterRef->GetProne())
			{
				if (const FST_CameraHeight * Row{ DT_CameraHeight->FindRow<FST_CameraHeight >("Prone", "") }) //22
				{
					NewHeight = Row->Height;
				}
			}
			else
			{
				if (!CharacterRef->GetProne() && !CharacterRef->GetCrouching())
				{
					if (const FST_CameraHeight * Row{ DT_CameraHeight->FindRow<FST_CameraHeight >("Stand", "") }) //100
					{
						NewHeight = Row->Height;
					}
				}
			}
			if (CurrentHeight != NewHeight) //Determine character is not standing, mean crouch or prone because _Currentheight of camera = 100 by default. In DataTable for stand pose is also 100 to set _NewHeight
			{
				//Set the original camera height to current, so original is taking the height from current height.
				OriginalHeight = CurrentHeight;
				//Start Timeline
				TL_CamHeightComponent->PlayFromStart();
			}
		}
	}
}

void ASurvivalPlayerController::TL_CamHeight_Update(float Percent)
{
	if (ASurvivalCharacter* CharacterRef = Cast<ASurvivalCharacter>(GetPawn()))
	{
		FVector CurrentLocation = CharacterRef->GetSpringArm()->GetRelativeLocation();
		float NewHeightR = FMath::Lerp(OriginalHeight, NewHeight, Percent); //Lerp
		CharacterRef->GetSpringArm()->SetRelativeLocation(FVector(CurrentLocation.X, CurrentLocation.Y, NewHeightR)); //Set Springarm Relative location
		CurrentHeight = NewHeightR;
	}
}

void ASurvivalPlayerController::StartFreeLook()
{
	ASurvivalCharacter* CharacterRef = Cast<ASurvivalCharacter>(GetPawn());
	checkf(IsValid(CharacterRef), TEXT("[%s] Invalid CharacterRef."), *FString(__FUNCTION__));

	//CharacterRef->StopAiming();
	CharacterRef->SetAltPressed(true);// = true;
	CharacterRef->SetAltPressedrotation(CharacterRef->GetSpringArm()->GetTargetRotation());
	CharacterRef->GetCharacterMovement()->bUseControllerDesiredRotation = false; //false
	CharacterRef->GetCharacterMovement()->bOrientRotationToMovement = true; //true
}
void ASurvivalPlayerController::StopFreeLook()
{
	ASurvivalCharacter* CharacterRef = Cast<ASurvivalCharacter>(GetPawn());
	checkf(IsValid(CharacterRef), TEXT("[%s] Invalid CharacterRef."), *FString(__FUNCTION__));

	CharacterRef->SetAltReleasedRotation(CharacterRef->GetSpringArm()->GetTargetRotation());
	CharacterRef->GetCharacterMovement()->bUseControllerDesiredRotation = true;
	CharacterRef->GetCharacterMovement()->bOrientRotationToMovement = false;
	TL_FreelookComponent->PlayFromStart();//Exe
}

FRotator ASurvivalPlayerController::GetControllerRotation()
{
	ASurvivalCharacter* CharacterRef = Cast<ASurvivalCharacter>(GetPawn());
	checkf(IsValid(CharacterRef), TEXT("[%s] Invalid CharacterRef."), *FString(__FUNCTION__));

	FRotator Rotation;
	FRotator ReturnRotation;
	if (CharacterRef->GetAltPressed())
	{
		ReturnRotation = CharacterRef->GetAltPressedRotation();
	}
	else
	{
		ReturnRotation = GetControlRotation();
	}
	Rotation = ReturnRotation;
	return Rotation;
}

void ASurvivalPlayerController::TL_FreeLook_Update(float Percentage)
{
	ASurvivalCharacter* CharacterRef = Cast<ASurvivalCharacter>(GetPawn());
	checkf(IsValid(CharacterRef), TEXT("[%s] Invalid CharacterRef."), *FString(__FUNCTION__));

	bool shortestPath = true;
	FRotator NewRotation(EForceInit::ForceInit);
	NewRotation = FRotator(FQuat::Slerp(FQuat(CharacterRef->GetAltReleasedRotation()), FQuat(CharacterRef->GetAltPressedRotation()), Percentage)); //Lerp Rotator = SLerp
	SetControlRotation(NewRotation);
	//return;
}

void ASurvivalPlayerController::TL_FreeLook_Finished()
{
	//ASurvivalCharacter* CharacterRef = Cast<ASurvivalCharacter>(GetPawn());
	//checkf(IsValid(CharacterRef), TEXT("[%s] Invalid CharacterRef."), *FString(__FUNCTION__));

	//CharacterRef->SetAltPressed(false);
}

#undef LOCTEXT_NAMESPACE