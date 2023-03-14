// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerupActor.h"
#include <Components/SphereComponent.h>

// Sets default values
ASPowerupActor::ASPowerupActor()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SphereComp->SetCollisionProfileName("Powerup");
	RootComponent = SphereComp;

	RespawnTime = 10.0f;
}

void ASPowerupActor::Interact_Implementation(APawn* InstigatorPawn)
{
	// logic in derived classes...
}

void ASPowerupActor::ShowPowerup()
{
	SetPowerupState(true);
}

void ASPowerupActor::HideAndCooldownPowerup()
{
	SetPowerupState(false);
	//timer to set power up state in CD = counting down timer before we call ShowPowerUp
	FTimerHandle TimerHandle_RespawnTimer;
	GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &ASPowerupActor::ShowPowerup, RespawnTime);
}

void ASPowerupActor::SetPowerupState(bool bNewIsActive)
{
	//no collision when it is false
	SetActorEnableCollision(bNewIsActive);

	// Set visibility on root and all children, invisible when false
	RootComponent->SetVisibility(bNewIsActive, true);
}