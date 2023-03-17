// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SBTService_CheckAttackRange.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

USBTService_CheckAttackRange::USBTService_CheckAttackRange()
{
	MaxAttackRange = 2000.f;
}

//need to add "AIModule", "GameplayTasks" in .build.cs module to make it work
void USBTService_CheckAttackRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	//check distance between AI pawn and Target Actor
	UBlackboardComponent* BlackBoardComp = OwnerComp.GetBlackboardComponent();
	if(ensure(BlackBoardComp))
	{
		//start with hard code
		//cast it first because the original return type is UObject
		AActor* TargetActor = Cast<AActor>(BlackBoardComp->GetValueAsObject("TargetActor"));
		//it is going to be nullptr many times, so dont ensure
		if(TargetActor)
		{
			AAIController* MyController = OwnerComp.GetAIOwner();
			//finally get AI Pawn
			APawn* AIPawn = MyController->GetPawn();

			if(ensure(AIPawn))
			{
				//use built-in distance function from FVector 
				float DistanceTo = FVector::Distance(TargetActor->GetActorLocation(), AIPawn->GetActorLocation());
				//hard code here
				bool bWithinRange = DistanceTo < MaxAttackRange;

				//use built-in line trace function to give AI some basic "sight"
				bool bHasLOS = false;
				//only check when Target Actor is in Range
				if(bWithinRange)
				{
					//return true if in sight
					bHasLOS = MyController->LineOfSightTo(TargetActor);
				}
				//put it in BlackBoard, and call .SelectedKeyName to get FName type
				BlackBoardComp->SetValueAsBool(AttackRangeKey.SelectedKeyName, bWithinRange && bHasLOS);
			}
		}
	}
}

