// Fill out your copyright notice in the Description page of Project Settings.


#include "SAttributeComponent.h"

#include "SGameModeBase.h"
#include "Net/UnrealNetwork.h"

//global damage multiplier in gm mode
static TAutoConsoleVariable<float> CVarDamageMultiplier(TEXT("su.DamageMultiplier"), 1.0f, TEXT("Global Damage Modifier for Attribute Component."), ECVF_Cheat);

// Sets default values for this component's properties
USAttributeComponent::USAttributeComponent()
{
	//setup default health
	HealthMax = 100;
	Health = HealthMax;

	Rage = 0;
	RageMax = 100;

	SetIsReplicatedByDefault(true);
}

bool USAttributeComponent::Kill(AActor* InstigatorActor)
{
	return ApplyHealthChange(InstigatorActor, -GetHealthMax());
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


float USAttributeComponent::GetHealth() const
{
	return Health;
}

float USAttributeComponent::GetHealthMax() const
{
	return HealthMax;
}

//dont use too generic name for InstigatorActor
bool USAttributeComponent::ApplyHealthChange(AActor* InstigatorActor,float Delta)
{
	//for god mode check if the actor can be damaged, use for debug purpose
	if (!GetOwner()->CanBeDamaged() && Delta < 0.0f)
	{
		return false;
	}

	//a very general way of multiplying damage, need better and specific versions of this
	if (Delta < 0.0f)
	{
		float DamageMultipler = CVarDamageMultiplier.GetValueOnGameThread();
		Delta *= DamageMultipler;
	}

	//current health stored in OldHealth
	float OldHealth = Health;

	//health is properly constrained between 0 and 100, so it stays at 0 even if we keep getting damages
	Health = FMath::Clamp(Health + Delta, 0.0f, HealthMax);

	//ActualDelta is the actual change of health of the character, and after character dies the delta would be 0
	float ActualDelta = Health - OldHealth;

	//OnHealthChanged.Broadcast(InstigatorActor, this, Health, ActualDelta); // @fixme: fixed!

	if (ActualDelta != 0.0f)
	{
		MulticastHealthChanged(InstigatorActor, Health, ActualDelta);
	}

	//died
	if (ActualDelta < 0.0f && Health == 0.0f)
	{
		//call GameModeBase class and only authority/server can call it 
		ASGameModeBase* GM = GetWorld()->GetAuthGameMode<ASGameModeBase>();
		if(GM)
		{
			//since its dead, whoever owns this attribute is killed by the Instigator
			GM->OnActorKilled(GetOwner(), InstigatorActor);
		}
	}

	return ActualDelta != 0;
}

float USAttributeComponent::GetRage() const
{
	return Rage;
}

bool USAttributeComponent::ApplyRage(AActor* InstigatorActor, float Delta)
{
	float OldRage = Rage;

	Rage = FMath::Clamp(Rage + Delta, 0.0f, RageMax);

	float ActualDelta = Rage - OldRage;
	if (ActualDelta != 0.0f)
	{
		OnRageChanged.Broadcast(InstigatorActor, this, Rage, ActualDelta);
	}

	return ActualDelta != 0;
}

USAttributeComponent* USAttributeComponent::GetAttributes(AActor* FromActor)
{
	if (FromActor)
	{
		//return components from this class
		return FromActor->FindComponentByClass<USAttributeComponent>();
		//return Cast<USAttributeComponent>(FromActor->GetComponentByClass(StaticClass()));
	}
	//default
	return nullptr;
}

bool USAttributeComponent::IsActorAlive(AActor* Actor)
{
	//get the attributes component
	USAttributeComponent* AttributeComp = GetAttributes(Actor);

	if(AttributeComp)
	{
		return AttributeComp->IsAlive();
	}

	//without an AttributeComp, should be "dead"
	return false;
}

void USAttributeComponent::MulticastHealthChanged_Implementation(AActor* InstigatorActor, float NewHealth, float Delta)
{
	OnHealthChanged.Broadcast(InstigatorActor, this, NewHealth, Delta);
}

void USAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USAttributeComponent, Health);
	DOREPLIFETIME(USAttributeComponent, HealthMax);
	//DOREPLIFETIME_CONDITION(USAttributeComponent, HealthMax, COND_InitialOnly);
}
