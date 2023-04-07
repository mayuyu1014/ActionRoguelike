// Fill out your copyright notice in the Description page of Project Settings.


#include "SMagicProjectile.h"
#include <Components/SphereComponent.h>

#include "SActionComponent.h"
#include "SActionEffect.h"
#include "SGameplayFunctionLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
ASMagicProjectile::ASMagicProjectile()
{

	SphereComp->SetSphereRadius(20.0f);
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ASMagicProjectile::OnActorOverlap);

	//inherited variable
	InitialLifeSpan = 10.0f;

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
		/*
			//Another method to do the parry,not recommended
			//since gameplay tag cant be created in C++
			//but we can always ask the manager if they have that tag stored
			FName Muzzle = "Muzzle_01";
			static FGameplayTag Tag = FGameplayTag::RequestGameplayTag("Status.Parrying");
		 */
		//when we are the "OtherActor"
		USActionComponent* ActionComp = Cast < USActionComponent > (OtherActor->GetComponentByClass(USActionComponent::StaticClass()));
		if (ActionComp && ActionComp->ActiveGameplayTags.HasTag(ParryTag))
		{
			//deflect the projectile 
			MoveComp->Velocity = -MoveComp->Velocity;
			//take ownership of the deflected projectile
			SetInstigator(Cast<APawn>(OtherActor));
			//need to return to skip the explode down there
			return;
		}

		//HitResult = SweepResult as projectile move by sweeping
		if(USGameplayFunctionLibrary::ApplyDirectionalDamage(GetInstigator(), OtherActor, DamageAmount, SweepResult))
		{
			Explode();

			//apply burning debuff, and applicable by the server
			if (ActionComp && HasAuthority())
			{
				ActionComp->AddAction(GetInstigator(), BurningActionClass);
			}
		}
	}
}

