// Fill out your copyright notice in the Description page of Project Settings.


#include "SAction_ProjectileAttack.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"


USAction_ProjectileAttack::USAction_ProjectileAttack()
{
	//copy from SCharacter, same setup
	//delay the spawn of attack projectile, set this up for avoiding hard coding
	AttackAnimDelay = 0.2f;

	HandSocketName = "Muzzle_01";
}

void USAction_ProjectileAttack::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);

	//UObject doesnt have some of the functions in Character class, so we cast Instigator to be a Character
	ACharacter* Character = Cast<ACharacter>(Instigator);
	if(Character)
	{
		//connect to the exsiting attack animations from contents, still needs to setup in blueprint
		Character->PlayAnimMontage(AttackAnim);

		//casting effects
		UGameplayStatics::SpawnEmitterAttached(CastingEffect, Character->GetMesh(), HandSocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget);

		//server only for this timer codes
		if (Character->HasAuthority())
		{
			//use a timer to delay the spawning of projectile to synchronize it with attack animation
			//A handler timer is a struct to hold the handle of this timer, pass as first parameter when set timer
			//PrimaryAttack_Elapsed as a functor, which actually implements the spawning of the projectile, after 0.2s
			FTimerHandle TimerHandle_AttackDelay;
			FTimerDelegate Delegate;
			Delegate.BindUFunction(this, "AttackDelay_Elapsed", Character);

			//we dont have WorldTimerManager since its a function in AActor, thus we call it seperately here
			GetWorld()->GetTimerManager().SetTimer(TimerHandle_AttackDelay, Delegate, AttackAnimDelay, false);
		}
	}
}

void USAction_ProjectileAttack::AttackDelay_Elapsed(ACharacter* InstigatorCharacter)
{
	//copied from SpawnProjectile function
	if (ensureAlways(ProjectileClass))
	{
		//find a specific location in the mesh - in this case, right hand
		FVector HandLocation = InstigatorCharacter->GetMesh()->GetSocketLocation(HandSocketName);

		//FActor is a struct holds a ton of parameters to use
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Instigator = InstigatorCharacter;

		FCollisionShape Shape;
		Shape.SetSphere(20.0f);

		// Ignore Player
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(InstigatorCharacter);

		//sweeping/looking for object types below
		FCollisionObjectQueryParams ObjParams;
		ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
		ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjParams.AddObjectTypesToQuery(ECC_Pawn);

		//get camera location from the character
		FVector TraceStart = InstigatorCharacter->GetPawnViewLocation();
		// endpoint far into the look-at distance (not too far, still adjust somewhat towards crosshair on a miss)
		FVector TraceEnd = TraceStart + (InstigatorCharacter->GetControlRotation().Vector() * 5000);

		FHitResult Hit;


		// true if we got to a blocking hit (Alternative: SweepSingleByChannel with ECC_WorldDynamic)
		if (GetWorld()->SweepSingleByObjectType(Hit, TraceStart, TraceEnd, FQuat::Identity, ObjParams, Shape, Params))
		{
			// Overwrite trace end with impact point in world
			TraceEnd = Hit.ImpactPoint;
		}

		// find new direction/rotation from Hand pointing to impact point in world.
		FRotator ProjRotation = FRotationMatrix::MakeFromX(TraceEnd - HandLocation).Rotator();


		//transform is a struct holds a location, rotation, and scale. here we pass in rotation and location
		FTransform SpawnTM = FTransform(ProjRotation, HandLocation);
		GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnTM, SpawnParams);
	}

	//stop after all the timer is done
	StopAction(InstigatorCharacter);
}