// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SGameplayInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USGameplayInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ACTIONROGUELIKE_API ISGameplayInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	//BlueprintNativeEvent tells the engine to expect a base class in C++ but still implementable in blueprint
	//BlueprintCallable allows us to call this function in blueprint

	//called after the actor state was restored from a save game file
	UFUNCTION(BlueprintNativeEvent)
	void OnActorLoaded();

	//display Interaction text, implemented in SPowerupActor
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FText GetInteractText(APawn* InstigatorPawn);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	//Interaction function between player (InstigatorPawn) and another object (AActor)
	void Interact(APawn* InstigatorPawn);
};
