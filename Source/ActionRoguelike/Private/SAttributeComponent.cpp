// Fill out your copyright notice in the Description page of Project Settings.


#include "SAttributeComponent.h"

// Sets default values for this component's properties
USAttributeComponent::USAttributeComponent()
{
	//setup default health
	HealthMax = 100;
	Health = HealthMax;
}

bool USAttributeComponent::IsAlive() const
{
	//alive when health > 0
	return Health > 0.0f;
}

bool USAttributeComponent::IsFullHealth() const
{
	return Health == HealthMax;
}

float USAttributeComponent::GetHealthMax() const
{
	return HealthMax;
}

bool USAttributeComponent::ApplyHealthChange(float Delta)
{
	//current health stored in OldHealth
	float OldHealth = Health;

	//health is properly constrained between 0 and 100, so it stays at 0 even if we keep getting damages
	Health = FMath::Clamp(Health + Delta, 0.0f, HealthMax);

	//ActualDelta is the actual damage the character gets in case HP is 0, we want to measure the actual damage not hp change
	float ActualDelta = Health - OldHealth;
	OnHealthChanged.Broadcast(nullptr, this, Health, ActualDelta); // @fixme: Still nullptr for InstigatorActor parameter

	return ActualDelta != 0;
}


