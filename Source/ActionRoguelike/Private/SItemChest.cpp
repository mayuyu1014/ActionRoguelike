// Fill out your copyright notice in the Description page of Project Settings.


#include "SItemChest.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASItemChest::ASItemChest()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//instantiate both parts of the chest
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	//bind base to the root
	RootComponent = BaseMesh;
	LidMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LidMesh"));
	//attach lid to the base
	LidMesh->SetupAttachment(BaseMesh);

	//setup pitch value
	TargetPitch = 110.0f;

	//synchronizing between server and client
	SetReplicates(true);
}

void ASItemChest::Interact_Implementation(APawn* InstigatorPawn)
{
	//changing bLidOpened everytime this function is called
	bLidOpened = !bLidOpened;

	//call the rep notify for the server
	OnRep_LidOpened();
}

//rep notify only auto trigger for client
void ASItemChest::OnRep_LidOpened()
{
	float CurrentPitch = bLidOpened ? TargetPitch : 0.0f;
	//relative rotation is relative to the thing it attaches, which is the base
	LidMesh->SetRelativeRotation(FRotator(CurrentPitch, 0, 0));
}

//replication rule
void ASItemChest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//whenever bLidOpened is true, send it to all clients
	DOREPLIFETIME(ASItemChest, bLidOpened);

}