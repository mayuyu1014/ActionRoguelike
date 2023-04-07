// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "GameFramework/GameModeBase.h"
#include "SGameModeBase.generated.h"

class UEnvQuery;
class UEnvQueryInstanceBlueprintWrapper;
class UCurveFloat;
class USSaveGame;
class UDataTable;
class USMonsterData;

//DataTable row for spawning monster in game mode
USTRUCT(BlueprintType)
struct FMonsterInfoRow : public FTableRowBase
{
	GENERATED_BODY()

	//default values of some of these variables
	FMonsterInfoRow()
	{
		Weight = 1.0f;
		SpawnCost = 5.0f;
		KillReward = 20.0f;
	}

	//specify the type of monster we want to spawn by using a asset manager
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPrimaryAssetId MonsterId;
	//referencing data asset instead of the class directly
	//USMonsterData* MonsterData; //hard reference
	//TSubclassOf<AActor> MonsterClass;

	//Relative chance to pick this monster
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Weight;

	//Points required by game mode to spawn this unit
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpawnCost;

	//Amount of credits awarded to killer of this unit
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float KillReward;
};

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ASGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

protected:

	FString SlotName;

	UPROPERTY()
	USSaveGame* CurrentSaveGame;

	//a data table for monsters instead of a class
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UDataTable* MonsterTable;

	//switch to data table
	//UPROPERTY(EditDefaultsOnly, Category = "AI")
	//TSubclassOf<AActor> MinionClass;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UEnvQuery* SpawnBotQuery;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	UCurveFloat* DifficultyCurve;

	FTimerHandle TimerHandle_SpawnBots;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float SpawnTimerInterval;

	// Read/write access as we could change this as our difficulty increases via Blueprint
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI")
	int32 CreditsPerKill;

	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	UEnvQuery* PowerupSpawnQuery;

	/* All power-up classes used to spawn with EQS at match start */
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	TArray<TSubclassOf<AActor>> PowerupClasses;

	/* Distance required between power-up spawn locations */
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	float RequiredPowerupDistance;

	/* Amount of powerups to spawn during match start */
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	int32 DesiredPowerupCount;

	UFUNCTION()
	void SpawnBotTimerElapsed();

	UFUNCTION()
	void OnBotSpawnQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus);

	UFUNCTION()
	void OnMonsterLoaded(FPrimaryAssetId LoadedId, FVector SpawnLocation);

	UFUNCTION()
	void OnPowerupSpawnQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus);

	UFUNCTION()
	void RespawnPlayerElapsed(AController* Controller);

public:

	//called in attribute class on ApplyOnHealthChange
	virtual void OnActorKilled(AActor* VictimActor, AActor* Killer);

	ASGameModeBase();

	/*
 * Initialize the game.
 * The GameMode's InitGame() event is called before any other functions (including PreInitializeComponents() )
 * and is used by the GameMode to initialize parameters and spawn its helper classes.
 * @warning: this is called before actors' PreInitializeComponents.
 */
	void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	virtual void StartPlay() override;

	/** Signals that a player is ready to enter the game, which may start it up */
	UFUNCTION(BlueprintNativeEvent, Category = Game)
	void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	//cheating code for debug purpose
	UFUNCTION(Exec)
	void KillAll();

	//available in BP make it easy to save game through UI or in game object 
	UFUNCTION(BlueprintCallable, Category="SaveGame")
	void WriteSaveGame();

	void LoadSaveGame();
};
