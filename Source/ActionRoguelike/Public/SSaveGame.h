// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SSaveGame.generated.h"

//this is the container holds all the actors' info
USTRUCT()
struct FActorSaveData
{
	GENERATED_BODY()

	//Identifier for which actor this belongs to
	UPROPERTY()
	FString ActorName;

	//For Movable actors, keep location, rotation, scale
	UPROPERTY()
	FTransform Transform;

	//this allows us to take any variable, convert to bytes, and store in this array
	UPROPERTY()
	TArray<uint8>ByteData;
};

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API USSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY()
	int32 Credits;

	UPROPERTY()
	TArray<FActorSaveData> SavedActors;
};
