// Fill out your copyright notice in the Description page of Project Settings.


#include "SInteractionComponent.h"

#include "SGameplayInterface.h"
#include "DrawDebugHelpers.h"

// Sets default values for this component's properties
USInteractionComponent::USInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void USInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void USInteractionComponent::PrimaryInteract()
{
	//the object type we detect is ECC_WorldDynamic
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	//this component class is attached to the character, so we call GetOwner() to get whoever it attaches to
	AActor* MyOwner = GetOwner();
	//and then we use character's built in function

	FVector EyeLocation;
	FRotator EyeRotation;
	//rotation is returned by camera rotation on the controller, eyelocation returns by default eyeheight of the character
	MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	//end of character's eyesight, convert rotation to vector(dirction it is looking) times 1000cm
	FVector End = EyeLocation + (EyeRotation.Vector() * 1000);

	/*
	FHitResult Hit;
	//use linetrace by object to dectect if theres any object to collide with, store value in bBlockingHit
	bool bBlockingHit = GetWorld()->LineTraceSingleByObjectType(Hit, EyeLocation, End, ObjectQueryParams);
	*/

	//need an array of hit results because we are sweeping for multiple overlap/blocks
	TArray<FHitResult> Hits;

	//use this variable to avoid hard code
	float radius = 30.0f;

	//the shape we use to sweep/scan the area
	FCollisionShape Shape;
	//set the Shape
	Shape.SetSphere(radius);

	//sweeping range/area to detect objects instead of a single line, Identity is a default/empty rotator
	bool bBlockingHit = GetWorld()->SweepMultiByObjectType(Hits, EyeLocation, End, FQuat::Identity, ObjectQueryParams, Shape);

	//check the bBlockingHit and coloring it
	FColor LineColor = bBlockingHit ? FColor::Green : FColor::Red;

	//get actor here to check its interface or other whatever properties
	//ranged for loop for Hits collection
	for (auto Hit : Hits) {
		AActor* HitActor = Hit.GetActor(); //without the loop this would be single-line tracing

		if (HitActor) //safety check pointer
		{
			//check if the HitActor is actually what we want to interact (if it is, it lives in that interface)
			//whenever we need to check if we implemented anything we use USGameplayInterface instead of I
			if (HitActor->Implements<USGameplayInterface>())
			{
				//cast MyOwner(AActor) to APawn, because the interact function specifies a pawn (InstigaterPawn)
				APawn* MyPawn = Cast<APawn>(MyOwner);
				//special syntax here to call interact function from the interface, and MyPawn can be null
				//when we need to actually implement things we use I
				ISGameplayInterface::Execute_Interact(HitActor, MyPawn);
				break; //find one hit and we are done, no need to sweep for more
			}
		}
		//debug sphere to expose the size of sphere we use to sweep, segments is the amount details we draw
		DrawDebugSphere(GetWorld(), Hit.ImpactPoint, radius, 32, LineColor, false, 2.0f);
	}

		//draw debug lines to expose the eyesight length and direction
		DrawDebugLine(GetWorld(), EyeLocation, End, LineColor, false, 2.0f, 0, 2.0f);
}

