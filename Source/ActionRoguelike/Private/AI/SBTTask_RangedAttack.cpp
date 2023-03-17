// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SBTTask_RangedAttack.h"

#include "AIController.h"
#include "SAttributeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"

USBTTask_RangedAttack::USBTTask_RangedAttack()
{
	//set default = good practice
	MaxBulletSpread = 2.0f;
}

//purpose of this function: spawn a projectile attack towards Target Actor
EBTNodeResult::Type USBTTask_RangedAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* MyController = OwnerComp.GetAIOwner();
	if(ensure(MyController))
	{
		//doing below to access GetMesh() method from Character class
		ACharacter* MyPawn = Cast<ACharacter>(MyController->GetPawn());
		if (MyPawn == nullptr)
		{
			return EBTNodeResult::Failed;
		}

		//Access BlackBoardComponent to get Target Actor
		AActor* TargetActor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject("TargetActor"));
		if (TargetActor == nullptr)
		{
			//fail until AI "sees" us
			return EBTNodeResult::Failed;
		}

		if (!USAttributeComponent::IsActorAlive(TargetActor))
		{
			//if target actor is not alive, the BT will be failed
			return EBTNodeResult::Failed;
		}

		//get a specific location of desired muzzle
		FVector MuzzleLocation = MyPawn->GetMesh()->GetSocketLocation("Muzzle_01");
		//subtract ActorLocation by MuzzleLocation to get the direction towards TargetActor
		FVector Direction = TargetActor->GetActorLocation() - MuzzleLocation;
		FRotator MuzzleRotation = Direction.Rotation();

		//make AI shooting less accurate, make sure dont use '=' to override it
		MuzzleRotation.Pitch += FMath::RandRange(0.0f, MaxBulletSpread);
		MuzzleRotation.Yaw += FMath::RandRange(-MaxBulletSpread, MaxBulletSpread);

		//Spawn the projectile attack, similar to ASCharacter
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		//to avoid hit itself
		Params.Instigator = MyPawn;

		AActor* NewProj = GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, MuzzleRotation, Params);
		return NewProj ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
	}
	//return Failed by default
	return EBTNodeResult::Failed;
}