// Fill out your copyright notice in the Description page of Project Settings.


#include "SMagicProjectile.h"
#include <Components/SphereComponent.h>
#include "SAttributeComponent.h"

// Sets default values
ASMagicProjectile::ASMagicProjectile()
{

	SphereComp->SetSphereRadius(20.0f);
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ASMagicProjectile::OnActorOverlap);

	DamageAmount = 20.0f;

	//below is no longer needed after inheriting from ProjectileBase class instead of AActor
	/*-----------------------
	//assign root to sphere
	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	
	//setup physics collision for this sphere, it is moving so WorldDynamic
	//SphereComp->SetCollisionObjectType(ECC_WorldDynamic);
	//apply settings to all channels
	//SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	//to one channel
	//SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	//setup custom for all projectiles to use the same setting, and it is easier to do it this way
	SphereComp->SetCollisionProfileName("Projectile"); //click project settings -> collision -> new
	//bind this projectile to make it deal damage
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ASMagicProjectile::OnActorOverlap);
	RootComponent = SphereComp;

	//attach particle effect to the sphere
	EffectComp = CreateDefaultSubobject<UParticleSystemComponent>("EffectComp");
	EffectComp->SetupAttachment(SphereComp);

	//some basic settings for the projectile
	MovementComp = CreateDefaultSubobject<UProjectileMovementComponent>("MovementComp");
	MovementComp->InitialSpeed = 1000.0f;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->bInitialVelocityInLocalSpace = true;
		------------------------*/
}

void ASMagicProjectile::OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//procedure: check if the actor is hit (check nullptr), if it is hit, get to the attribute component and call health change function
	if (OtherActor && OtherActor != GetInstigator()) //dont wanna hurt ourself
	{
		//GetComponentByClass will iterate through all class components in OtherActor
		//this function returns UActorComponent, so we need to cast it to USAttributeComponent
		USAttributeComponent* AttributeComp = Cast<USAttributeComponent>(OtherActor->GetComponentByClass(USAttributeComponent::StaticClass()));

		//again, check AttributeComp, because it could be nullptr, like if we hit a wall
		if (AttributeComp)
		{
			//if it is not nullptr, that means we hit some somebody, and we deal some damage to its health
			AttributeComp->ApplyHealthChange(-DamageAmount);

			// Only explode when we hit something valid
			Explode();

			//update below to the avoveExplode method
			/*
			//after dealing damage, projectile should be removed
			Destroy();
			*/
		}

	}
}

