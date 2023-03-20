// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameplayFunctionLibrary.h"

#include "SAttributeComponent.h"


//apply damage for global use, so we dont have to update AttributeComponent every time 
bool USGameplayFunctionLibrary::ApplyDamage(AActor* DamageCauser, AActor* TargetActor, float DamageAmount)
{
	USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(TargetActor);

	if(AttributeComp)
	{
		return AttributeComp->ApplyHealthChange(DamageCauser, -DamageAmount);
	}
	return false;
}

bool USGameplayFunctionLibrary::ApplyDirectionalDamage(AActor* DamageCauser, AActor* TargetActor, float DamageAmount, const FHitResult& HitResult)
{
	//only happens when damage is successfully dealt
	if (ApplyDamage(DamageCauser, TargetActor, DamageAmount))
	{
		//apply an impulse onto a specific part of mesh where projectile or attacks hit
		UPrimitiveComponent* HitComp = HitResult.GetComponent();

		if (HitComp && HitComp->IsSimulatingPhysics(HitResult.BoneName))
		{
			//To get the proper shooting direction, we need a trace as a vector
			//Direction = Target - Origin
			FVector Direction = HitResult.TraceEnd - HitResult.TraceStart;
			//need to normalize() it to make it a proper directional vector
			Direction.Normalize();

			//ImpactNormal is the direction from the attacker, so need to invert it
			HitComp->AddImpulseAtLocation(Direction * 150000.f, HitResult.ImpactPoint, HitResult.BoneName);
		}
		return true;
	}

	return false;
}