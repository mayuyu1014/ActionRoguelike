// Fill out your copyright notice in the Description page of Project Settings.


#include "SInteractionComponent.h"

#include "SGameplayInterface.h"
#include "DrawDebugHelpers.h"
#include "SWorldUserWidget.h"

//a console variable to control the on/off of debug drawing tool
static TAutoConsoleVariable<bool> CVarDebugDrawInteraction(TEXT("su.InteractionDebugDraw"), false, TEXT("Enable Debug Line for Interact Component."), ECVF_Cheat);

// Sets default values for this component's properties
USInteractionComponent::USInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	TraceRadius = 30.0f;
	TraceDistance = 500.0f;
	CollisionChannel = ECC_WorldDynamic;
}

// Called when the game starts
void USInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void USInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//in multiplayer, find the player who actually needs to run the code
	APawn* MyPawn = Cast<APawn>(GetOwner());
	if(MyPawn->IsLocallyControlled())
	{
		//check it every tick
		FindBestInteractable();
	}
}

void USInteractionComponent::FindBestInteractable()
{
	//debug drawing tool switch
	bool bDebugDraw = CVarDebugDrawInteraction.GetValueOnGameThread();

	//the object type we detect is ECC_WorldDynamic
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(CollisionChannel);

	//this component class is attached to the character, so we call GetOwner() to get whoever it attaches to
	AActor* MyOwner = GetOwner();
	//and then we use character's built in function

	FVector EyeLocation;
	FRotator EyeRotation;
	//rotation is returned by camera rotation on the controller, eyelocation returns by default eyeheight of the character
	MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	//end of character's eyesight, convert rotation to vector(dirction it is looking) times 1000cm
	FVector End = EyeLocation + (EyeRotation.Vector() * TraceDistance);

	/*
	FHitResult Hit;
	//use linetrace by object to dectect if theres any object to collide with, store value in bBlockingHit
	bool bBlockingHit = GetWorld()->LineTraceSingleByObjectType(Hit, EyeLocation, End, ObjectQueryParams);
	*/

	//need an array of hit results because we are sweeping for multiple overlap/blocks
	TArray<FHitResult> Hits;

	//the shape we use to sweep/scan the area
	FCollisionShape Shape;
	//set the Shape
	Shape.SetSphere(TraceRadius);

	//sweeping range/area to detect objects instead of a single line, Identity is a default/empty rotator
	bool bBlockingHit = GetWorld()->SweepMultiByObjectType(Hits, EyeLocation, End, FQuat::Identity, ObjectQueryParams, Shape);

	//check the bBlockingHit and coloring it
	FColor LineColor = bBlockingHit ? FColor::Green : FColor::Red;

	//declare FocusedActor before try to fill it with sweeping data
	FocusedActor = nullptr;

	//get actor here to check its interface or other whatever properties
	//ranged for loop for Hits collection
	for (auto Hit : Hits) {

		//draw debug line if switch is on
		if (bDebugDraw)
		{
			//debug sphere to expose the size of sphere we use to sweep, segments is the amount details we draw
			DrawDebugSphere(GetWorld(), Hit.ImpactPoint, TraceRadius, 32, LineColor, false, 0.0f);
		}

		AActor* HitActor = Hit.GetActor(); //without the loop this would be single-line tracing

		if (HitActor) //safety check pointer
		{
			//check if the HitActor is actually what we want to interact (if it is, it lives in that interface)
			//whenever we need to check if we implemented anything we use USGameplayInterface instead of I
			if (HitActor->Implements<USGameplayInterface>())
			{
				//when we find something interactable, keep track of it
				FocusedActor = HitActor;
				break; //find one hit and we are done, no need to sweep for more
			}
		}
	}
	//instead of make it in BeginPlay(), do it here mean we create this lazily = create only when need it
	if (FocusedActor)
	{
		if (DefaultWidgetInstance == nullptr && ensure(DefaultWidgetClass))
		{
			DefaultWidgetInstance = CreateWidget<USWorldUserWidget>(GetWorld(), DefaultWidgetClass);
		}

		//when instance is successfully created
		if (DefaultWidgetInstance)
		{
			//attach it
			DefaultWidgetInstance->AttachedActor = FocusedActor;
			//only add when it is not in viewport
			if (!DefaultWidgetInstance->IsInViewport())
			{
				DefaultWidgetInstance->AddToViewport();
			}
		}
	}else
	{
		if(DefaultWidgetInstance)
		{
			DefaultWidgetInstance->RemoveFromParent();
		}
	}

	if (bDebugDraw)
	{
		//draw debug lines to expose the eyesight length and direction
		DrawDebugLine(GetWorld(), EyeLocation, End, LineColor, false, 2.0f, 0, 0.0f);
	}
}

void USInteractionComponent::PrimaryInteract()
{
	ServerInteract(FocusedActor);
}

void USInteractionComponent::ServerInteract_Implementation(AActor* InFocus)
{
	//check FocusedActor or program crash
	if (InFocus == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, "No Focused Actor to Interact");
		return;
	}

	//cast MyOwner(AActor) to APawn, because the interact function specifies a pawn (InstigaterPawn)
	APawn* MyPawn = Cast<APawn>(GetOwner());
	//special syntax here to call interact function from the interface, and MyPawn can be null
	//when we need to actually implement things we use I
	ISGameplayInterface::Execute_Interact(InFocus, MyPawn);
}
