//Extended verison 2023 By [Yalenka] - SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "InteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginInteract, class ASurvivalCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndInteract, class ASurvivalCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginFocus, class ASurvivalCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndFocus, class ASurvivalCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteract, class ASurvivalCharacter*, Character);

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SURVIVALGAME_API UInteractionComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:

	UInteractionComponent();

	//The time the player must hold the interact key to interact with this object
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionTime;

	//The max distance the player can be away from this actor before you can interact
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionDistance;

	//The name that will come up when the player looks at the interactable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText InteractableNameText;

	//The verb that describes how the interaction works, ie "Sit" for a chair, "Eat" for food, "Light" for a fireplace
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText InteractableActionText;

	//Whether we allow multiple players to interact with the item, or just one at any given time.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bAllowMultipleInteractors;

	//Call this to change the name of the interactable. Will also refresh the interaction widget.
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractableNameText(const FText& NewNameText);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractableActionText(const FText& NewActionText);

	//Delegates

	//[local + server] Called when the player presses the interact key whilst focusing on this interactable actor
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnBeginInteract OnBeginInteract;

	//[local + server] Called when the player releases the interact key, stops looking at the interactable actor, or gets too far away after starting an interact
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnEndInteract OnEndInteract;

	//[local + server] Called when the player presses the interact key whilst focusing on this interactable actor
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnBeginFocus OnBeginFocus;

	//[local + server] Called when the player releases the interact key, stops looking at the interactable actor, or gets too far away after starting an interact
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnEndFocus OnEndFocus;

	//[local + server] Called when the player has interacted with the item for the required amount of time
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnBeginInteract OnInteract;

protected:
	// Called when the game starts
	virtual void Deactivate() override;

	bool CanInteract(class ASurvivalCharacter* Character) const;

	//On the server, this will hold all interactors. On the local player, this will just hold the local player (provided they are an interactor)
	UPROPERTY()
	TArray<class ASurvivalCharacter*> Interactors;

public:

	/***Refresh the interaction widget and its custom widgets.
	An example of when we'd use this is when we take 3 items out of a stack of 10, and we need to update the widget
	so it shows the stack as having 7 items left. */
	void RefreshWidget();

	//Called on the client when the players interaction check trace begins/ends hitting this item
	void BeginFocus(class ASurvivalCharacter* Character);
	void EndFocus(class ASurvivalCharacter* Character);

	//Called on the client when the player begins/ends interaction with the item
	void BeginInteract(class ASurvivalCharacter* Character);
	void EndInteract(class ASurvivalCharacter* Character);

	void Interact(class ASurvivalCharacter* Character);

	//Return a value from 0-1 denoting how far through the interact we are. 
	//On server this is the first interactors percentage, on client this is the local interactors percentage
	UFUNCTION(BlueprintPure, Category = "Interaction")
	float GetInteractPercentage();

};
