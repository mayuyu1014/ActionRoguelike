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
	FName ActorName;

	//For Movable actors, keep location, rotation, scale
	UPROPERTY()
	FTransform Transform;

	//this allows us to take any variable, convert to bytes, and store in this array
	UPROPERTY()
	TArray<uint8>ByteData;
};

USTRUCT()
struct FPlayerSaveData
{
	GENERATED_BODY()

	//Player Id defined by the online sub system(such as steam) converted to FString for simplicity
	UPROPERTY()
	FString PlayerID;

	UPROPERTY()
	int32 Credits;

	//Longest survival time
	UPROPERTY()
	float PersonalRecordTime;

	//Location if player was alive during save
	UPROPERTY()
	FVector Location;

	//Orientation if player was alive during save
	UPROPERTY()
	FRotator Rotation;

	//we dont always want to restor location, and may just resume player at specific respawn point in world.
	UPROPERTY()
	bool bResumeAtTransform;
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
	TArray<FPlayerSaveData> SavedPlayers;

	//Actors stored from a level (currently does not support a specific level and just assumes the demo map)
	UPROPERTY()
	TArray<FActorSaveData> SavedActors;

	FPlayerSaveData* GetPlayerData(APlayerState* PlayerState);
};
