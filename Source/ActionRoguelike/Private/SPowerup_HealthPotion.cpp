// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerup_HealthPotion.h"

#include "SAttributeComponent.h"

ASPowerup_HealthPotion::ASPowerup_HealthPotion()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	// Disable collision, instead we use SphereComp to handle interaction queries
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetupAttachment(RootComponent);
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
		// Only activate if healed successfully, and heal the character to full health
		//change this parameter to play around the amount of healing it can be done
		if (AttributeComp->ApplyHealthChange(this, AttributeComp->GetHealthMax()))
		{
			HideAndCooldownPowerup();
		}
	}
}