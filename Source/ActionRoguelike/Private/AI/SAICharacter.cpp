// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SAICharacter.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "SAttributeComponent.h"
#include "SWorldUserWidget.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ASAICharacter::ASAICharacter()
{
	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>("PawnSensingComp");

	AttributeComp = CreateDefaultSubobject<USAttributeComponent>("AttributeComp");

	//let the controller auto possess AICharacters after they spawn, everything here is built in parent
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	//collision modification so minion mesh can be properly hit
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);

	TimeToHitParamName = "TimeToHit";
}

void ASAICharacter::PostInitializeComponents()
{
	//dont forget this
	Super::PostInitializeComponents();

	//binding to the delegate function
	PawnSensingComp->OnSeePawn.AddDynamic(this, &ASAICharacter::OnPawnSeen);

	AttributeComp->OnHealthChanged.AddDynamic(this, &ASAICharacter::OnHealthChanged);
}

//copied/borrowed from SCharacter 
void ASAICharacter::OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	//check for damage not healing
	if (Delta < 0.0f)
	{
		//dont hit ourselves(AI) check
		if(InstigatorActor != this)
		{
			SetTargetActor(InstigatorActor);
		}

		//only run below when there is no active health bar 
		if (ActiveHealthBar == nullptr) {
			//spawn the health bar widget made for this AI minion
			ActiveHealthBar = CreateWidget<USWorldUserWidget>(GetWorld(), HealthBarWidgetClass);
			//add it to the viewport
			if (ActiveHealthBar)
			{
				//bind the attributes to this widget, and do this before AddToViewport
				ActiveHealthBar->AttachedActor = this;
				//and add it to viewport
				ActiveHealthBar->AddToViewport();
			}
		}

		//HitFlash component
		GetMesh()->SetScalarParameterValueOnMaterials(TimeToHitParamName, GetWorld()->TimeSeconds);

		//dead
		if (NewHealth <= 0.0f)
		{
			//at the moment AI dies, we want to:
			//Stop BT, we can get it from AI controller
			AAIController* AIC = Cast<AAIController>(GetController());
			if (AIC)
			{
				//AI brain that controls the BT, and reason is just for debugging
				AIC->GetBrainComponent()->StopLogic("Killed");
			}

			//Ragdoll - get the Mesh of the AI and change its collision profile at run time
			GetMesh()->SetAllBodiesSimulatePhysics(true);
			//if we dont change this, the minion body would just fall through the floor
			GetMesh()->SetCollisionProfileName("Ragdoll");

			//remove collision after dead
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			//also stop movement
			GetCharacterMovement()->DisableMovement();

			//Set Lifespan -  how long we call destroy() on it
			SetLifeSpan(10.0f);
		}
	}
}

void ASAICharacter::SetTargetActor(AActor* NewTarget)
{
	//get AIcontroller, and cast it to AAIController type
	AAIController* AIC = Cast<AAIController>(GetController());

	if (AIC)
	{
		//access to BlackBoard component and bind
		AIC->GetBlackboardComponent()->SetValueAsObject("TargetActor", NewTarget);
	}
}

void ASAICharacter::OnPawnSeen(APawn* Pawn)
{
	SetTargetActor(Pawn);

		//debug helper
	DrawDebugString(GetWorld(), GetActorLocation(), "Player Spotted", nullptr, FColor::White, 4.0f, true);
}


