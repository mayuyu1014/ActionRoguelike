// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"


void ASAIController::BeginPlay()
{
	Super::BeginPlay();

	if (ensureMsgf(BehaviorTree, TEXT("Behavior Tree is nullptr! Please assign BehaviorTree in your AI Controller.")))
	{
		//alternative to ensure, we have UE version of assert
		if (ensureMsgf(BehaviorTree, TEXT("Behavior Tree is nullptr. Please assign BehaviorTree in your AI Controller"))) {
			RunBehaviorTree(BehaviorTree);
		}
	}
	/*
	 //old method, now we only want actor be seen as target actor
	//GetPlayerPawn needs us to pass any player that lives in the world, with its player index, which is our character here
	APawn* MyPawn = UGameplayStatics::GetPlayerPawn(this, 0);

	//safery check first
	if (MyPawn)
	{
		//call the move to location in blackboard memory and assign the location to be where player spawns
		GetBlackboardComponent()->SetValueAsVector("MoveToLocation", MyPawn->GetActorLocation());

		//assign the location to where the TargetActor is
		GetBlackboardComponent()->SetValueAsObject("TargetActor", MyPawn);
	}
	*/
}
