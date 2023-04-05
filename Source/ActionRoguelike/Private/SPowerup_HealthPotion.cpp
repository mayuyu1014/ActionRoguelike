// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerup_HealthPotion.h"

#include "SAttributeComponent.h"
#include "SPlayerState.h"


//remember to undef this
#define LOCTEXT_NAMESPACE "InteractableActors"

ASPowerup_HealthPotion::ASPowerup_HealthPotion()
{
	CreditCost = 50;
}

void ASPowerup_HealthPotion::Interact_Implementation(APawn* InstigatorPawn)
{
	if (!ensure(InstigatorPawn))
	{
		return;
	}

	USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(InstigatorPawn);
	//old cast method below
	//USAttributeComponent* AttributeComp = Cast<USAttributeComponent>(InstigatorPawn->GetComponentByClass(USAttributeComponent::StaticClass()));

	// Check if not already at max health, not necessary, can be removed
	if (ensure(AttributeComp) && !AttributeComp->IsFullHealth())
	{
		//check pointer safety
		if (ASPlayerState* PS = InstigatorPawn->GetPlayerState<ASPlayerState>())
		{
			// Only activate if healed successfully, and heal the character to full health
			//change this parameter to play around the amount of healing it can be done
			//remove credits before healing
			if (PS->RemoveCredits(CreditCost) && AttributeComp->ApplyHealthChange(this, AttributeComp->GetHealthMax()))
				{
					HideAndCooldownPowerup();
				}
		}
	}
}

FText ASPowerup_HealthPotion::GetInteractText_Implementation(APawn* InstigatorPawn)
{
	USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(InstigatorPawn);
	if (AttributeComp && AttributeComp->IsFullHealth())
	{
		//display this message at full health
		return LOCTEXT("HealthPotion_FullHealthWarning", "Already at full health");
	}

	//defaul message
	return FText::Format(LOCTEXT("HealthPotion_InteractMessage", "Cost {0} Credits. Restore health to maximum."), CreditCost);
}

#undef LOCTEXT_NAMESPACE