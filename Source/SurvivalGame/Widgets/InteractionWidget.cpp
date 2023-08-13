//Extended verison 2023 By [Yalenka] - SurvivalGame Project - The Unreal C++ Survival Game Course - Copyright Reuben Ward 2020


#include "InteractionWidget.h"
#include "Components/InteractionComponent.h"

void UInteractionWidget::UpdateInteractionWidget(class UInteractionComponent* InteractionComponent)
{
	OwningInteractionComponent = InteractionComponent;
	OnUpdateInteractionWidget();
}
