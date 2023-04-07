// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameModeBase.h"

#include "EngineUtils.h"
#include "SAttributeComponent.h"
#include "AI/SAICharacter.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "DrawDebugHelpers.h"
#include "SActionComponent.h"
#include "SCharacter.h"
#include "SGameplayInterface.h"
#include "SMonsterData.h"
#include "SPlayerState.h"
#include "SSaveGame.h"
#include "ActionRoguelike/ActionRoguelike.h"
#include "Engine/AssetManager.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

//console variables for debug, ECVF_Cheat marks this function to not be included in the final build
static TAutoConsoleVariable<bool>CVarSpawnBots(TEXT("su.SpawnBots"), true, TEXT("Enable spawning of bots via timer."), ECVF_Cheat);


ASGameModeBase::ASGameModeBase()
{
	SpawnTimerInterval = 2.0f;
	CreditsPerKill = 20;

	DesiredPowerupCount = 10;
	RequiredPowerupDistance = 2000;

	PlayerStateClass = ASPlayerState::StaticClass();

	SlotName = "SaveGame_01";
}

void ASGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	//load saved data before everything else
	LoadSaveGame();
}

void ASGameModeBase::StartPlay()
{
	//dont forget this
	Super::StartPlay();

	//Continuous timer to spawn in more bots.
	//Actual amount of bots and whether its allowed to spawn determined by spawn logic later in the chain...
	//setup a timer to call the spawn query, and find a spawn spot
	GetWorldTimerManager().SetTimer(TimerHandle_SpawnBots, this, &ASGameModeBase::SpawnBotTimerElapsed, SpawnTimerInterval, true);

	// Make sure we have assigned at least one power-up class
	if (ensure(PowerupClasses.Num() > 0))
	{
		// Run EQS to find potential power-up spawn locations
		UEnvQueryInstanceBlueprintWrapper* QueryInstance = UEnvQueryManager::RunEQSQuery(this, PowerupSpawnQuery, this, EEnvQueryRunMode::AllMatching, nullptr);
		if (ensure(QueryInstance))
		{
			QueryInstance->GetOnQueryFinishedEvent().AddDynamic(this, &ASGameModeBase::OnPowerupSpawnQueryCompleted);
		}
	}
}

void ASGameModeBase::HandleStartingNewPlayer_Implementation_Implementation(APlayerController* NewPlayer)
{
	// Calling Before Super:: so we set variables before 'beginplayingstate' is called in PlayerController (which is where we instantiate UI)
	//update player state for newly joined player
	ASPlayerState* PS = NewPlayer->GetPlayerState<ASPlayerState>();
	if (PS)
	{
		PS->LoadPlayerState(CurrentSaveGame);
	}

	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
};

void ASGameModeBase::KillAll()
{
	for (ASAICharacter* Bot : TActorRange<ASAICharacter>(GetWorld()))
	{
		USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(Bot);
		if (AttributeComp && AttributeComp->IsAlive())
		{
			AttributeComp->Kill(this); //@fixme: passing player for kill credit?
		}
	}
}

void ASGameModeBase::SpawnBotTimerElapsed()
{
	if (!CVarSpawnBots.GetValueOnGameThread())
	{
		UE_LOG(LogTemp, Warning, TEXT("Bot spawning disabled via cvar 'CVarSpawnBots'"));
		return;
	}

	//check if we hit the spawn limit before actually spawn any bots
	//limiting the amount of AI minions can be spawned in a level
	int32 NrOfAliveBots = 0;
	for (ASAICharacter* Bot : TActorRange<ASAICharacter>(GetWorld()))
	{
		USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(Bot);
		if (AttributeComp)
		{
			NrOfAliveBots++;
		}
	}

	//Debug message to ourselves
	UE_LOG(LogTemp, Log, TEXT("Found %i alive bots."), NrOfAliveBots);

	//stop generating more bots after 5 bots existing
	float MaxBotCount = 5.0f;

	//use this curve to control the number of bots spawning in different time
	if (DifficultyCurve)
	{
		//the curve is made and designed in BP
		MaxBotCount = DifficultyCurve->GetFloatValue(GetWorld()->TimeSeconds);
	}

	if (NrOfAliveBots >= MaxBotCount)
	{
		//Debug message to ourselves
		UE_LOG(LogTemp, Log, TEXT("At Maximum Capacity, Skipping Bot Spawn."));
		return;
	}

	//get the instance of the query to have the trigger for the events
	UEnvQueryInstanceBlueprintWrapper* QueryInstance = UEnvQueryManager::RunEQSQuery(this, SpawnBotQuery, this, EEnvQueryRunMode::RandomBest5Pct, nullptr);
	//call the macro to bind the trigger function
	//and safety check is important here
	if (ensure(QueryInstance))
	{
		QueryInstance->GetOnQueryFinishedEvent().AddDynamic(this, &ASGameModeBase::OnBotSpawnQueryCompleted);
	}
}

void ASGameModeBase::OnBotSpawnQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus)
{
	//safety check
	if (QueryStatus != EEnvQueryStatus::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawn Bot EQS Query Failed!"));
		return;
	}

	//get the locations to spawn bots
	TArray<FVector> Locations = QueryInstance->GetResultsAsLocations();

	//check if there is any valid location
	if (Locations.IsValidIndex(0))
	{
		if (MonsterTable)
		{
			TArray<FMonsterInfoRow*> Rows;
			//first parameter doesnt matter that much
			MonsterTable->GetAllRows("", Rows);

			//get random enemy from those rows
			int32 RandomIndex = FMath::RandRange(0, Rows.Num() - 1);
			FMonsterInfoRow* SelectedRow = Rows[RandomIndex];

			//create an asset manager to contain the soft references to the actual moster data
			UAssetManager* Manager = UAssetManager::GetIfValid();
			if (Manager)
			{
				LogOnScreen(this, "Loading monster...", FColor::Green);

				//empty set of bundles of soft references, later on we can add all the soft pointers to it
				TArray<FName> Bundles;

				//this is not BP available but can dynamically adding parameters
				FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this, &ASGameModeBase::OnMonsterLoaded, SelectedRow->MonsterId, Locations[0]);

				Manager->LoadPrimaryAsset(SelectedRow->MonsterId, Bundles, Delegate);
			}
		} 
		//track all the used spawn locations
		//DrawDebugSphere(GetWorld(), Locations[0], 50.0f, 20, FColor::Blue, false, 60.0f);
	}
}

//the actual monster loading data
void ASGameModeBase::OnMonsterLoaded(FPrimaryAssetId LoadedId, FVector SpawnLocation)
{
	//LogOnScreen(this, "Finished loading.", FColor::Green);

	UAssetManager* Manager = UAssetManager::GetIfValid();
	if (Manager)
	{
		USMonsterData* MonsterData = Cast<USMonsterData>(Manager->GetPrimaryAssetObject(LoadedId));
		if (MonsterData)
		{
			//spawning bots with action component
			AActor* NewBot = GetWorld()->SpawnActor<AActor>(MonsterData->MonsterClass, SpawnLocation, FRotator::ZeroRotator);
			if (NewBot)
			{
				LogOnScreen(this, FString::Printf(TEXT("Spawned enemy: %s (%s)"), *GetNameSafe(NewBot), *GetNameSafe(MonsterData)));

				//Grant special actions, buffs, etc.
				USActionComponent* ActionComp = Cast<USActionComponent>(NewBot->GetComponentByClass(USActionComponent::StaticClass()));
				if (ActionComp)
				{
					for (TSubclassOf<USAction> ActionClass : MonsterData->Actions)
					{
						ActionComp->AddAction(NewBot, ActionClass);
					}
				}
			}
		}
	}
}

void ASGameModeBase::OnPowerupSpawnQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus)
{
	if (QueryStatus != EEnvQueryStatus::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawn bot EQS Query Failed!"));
		return;
	}

	TArray<FVector> Locations = QueryInstance->GetResultsAsLocations();

	// Keep used locations to easily check distance between points
	TArray<FVector> UsedLocations;

	int32 SpawnCounter = 0;
	// Break out if we reached the desired count or if we have no more potential positions remaining

	while (SpawnCounter < DesiredPowerupCount && Locations.Num() > 0)
	{
		// Pick a random location from remaining points.
		int32 RandomLocationIndex = FMath::RandRange(0, Locations.Num() - 1);

		FVector PickedLocation = Locations[RandomLocationIndex];
		// Remove to avoid picking again
		Locations.RemoveAt(RandomLocationIndex);

		// Check minimum distance requirement
		bool bValidLocation = true;

		for (FVector OtherLocation : UsedLocations)
		{
			float DistanceTo = (PickedLocation - OtherLocation).Size();

			if (DistanceTo < RequiredPowerupDistance)
			{
				// Show skipped locations due to distance
				//DrawDebugSphere(GetWorld(), PickedLocation, 50.0f, 20, FColor::Red, false, 10.0f);

				// too close, skip to next attempt
				bValidLocation = false;
				break;
			}
		}

		// Failed the distance test
		if (!bValidLocation)
		{
			continue;
		}

		// Pick a random powerup-class
		int32 RandomClassIndex = FMath::RandRange(0, PowerupClasses.Num() - 1);
		TSubclassOf<AActor> RandomPowerupClass = PowerupClasses[RandomClassIndex];

		GetWorld()->SpawnActor<AActor>(RandomPowerupClass, PickedLocation, FRotator::ZeroRotator);

		// Keep for distance checks
		UsedLocations.Add(PickedLocation);
		SpawnCounter++;
	}
}

void ASGameModeBase::RespawnPlayerElapsed(AController* Controller)
{
	if (ensure(Controller))
	{
		//unbind the controller will detach the player from us, we need to call this before respawn
		Controller->UnPossess();

		//built in function that teleports player character to where game starts
		RestartPlayer(Controller);
	}
}

//need this to respawn the player
void ASGameModeBase::OnActorKilled(AActor* VictimActor, AActor* Killer)
{
	UE_LOG(LogTemp, Log, TEXT("OnActorKilled: Victim: %s, Killer: %s"), *GetNameSafe(VictimActor), *GetNameSafe(Killer));

	// Respawn Players after delay
	ASCharacter* Player = Cast<ASCharacter>(VictimActor);
	if (Player)
	{
		//make this a local variable to avoid it being override in multiplayer game
		FTimerHandle TimerHandle_RespawnDelay;

		//who should respawn?
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "RespawnPlayerElapsed", Player->GetController());

		float RespawnDelay = 2.0f;
		//set a timer before respawn the player
		GetWorldTimerManager().SetTimer(TimerHandle_RespawnDelay, Delegate, RespawnDelay, false);
	}

	// Give Credits for kill
	APawn* KillerPawn = Cast<APawn>(Killer);
	if (KillerPawn && KillerPawn != VictimActor)
	{
		//Only Players will have a 'PlayerState' instance, bots have nullptr
		ASPlayerState* PS = KillerPawn->GetPlayerState<ASPlayerState>();
		if (PS)
		{
			PS->AddCredits(CreditsPerKill);
		}
	}
}

void ASGameModeBase::WriteSaveGame()
{
	//Iterate all player states, we dont have proper ID to match yet (requires steam or EOS
	for (int32 i=0; i<GameState->PlayerArray.Num(); i++)
	{
		ASPlayerState* PS = Cast<ASPlayerState>(GameState->PlayerArray[i]);
		if(PS)
		{
			PS->SavePlayerState(CurrentSaveGame);
			break; //break here because single player
		}
	}

	//clear the container before adding actor
	CurrentSaveGame->SavedActors.Empty();

	//iterator all the actors in the world and save them
	for (FActorIterator It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		//only interested in our gameplay actors
		if (!Actor->Implements<USGameplayInterface>())
		{
			continue;
		}

		//fill the struct/container with data
		FActorSaveData ActorData;
		ActorData.ActorName = Actor->GetName();
		ActorData.Transform = Actor->GetActorTransform();

		//pass the array to fill with data from Actor
		FMemoryWriter MemWriter(ActorData.ByteData);
		//need this for the "SaveGame" variables
		FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
		//find only variables with UPROPERTY(SaveGame)
		Ar.ArIsSaveGame = true;
		//Convert Actor's SaveGame UPROPERTY into binary array
		Actor->Serialize(Ar);

		CurrentSaveGame->SavedActors.Add(ActorData);
	}

	UGameplayStatics::SaveGameToSlot(CurrentSaveGame, SlotName, 0);
}

void ASGameModeBase::LoadSaveGame()
{
	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		CurrentSaveGame = Cast<USSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
		if (CurrentSaveGame == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to Load SaveGame Data."));
			return;
		}
		UE_LOG(LogTemp, Log, TEXT("Loaded SaveGame Data."));

		//do this after loading complete
		//iterator all the actors in the world and save them
		for (FActorIterator It(GetWorld()); It; ++It)
		{
			AActor* Actor = *It;
			//only interested in our gameplay actors, implementing interface, so use 'U' prefix
			if (!Actor->Implements<USGameplayInterface>())
			{
				continue;
			}

			//find matches
			for (FActorSaveData ActorData : CurrentSaveGame->SavedActors)
			{
				if (ActorData.ActorName == Actor->GetName())
				{
					Actor->SetActorTransform(ActorData.Transform);

					//reverse the saving process
					FMemoryReader MemReader(ActorData.ByteData);
					FObjectAndNameAsStringProxyArchive Ar(MemReader, true);
					Ar.ArIsSaveGame = true;
					//conver binary data back to Actor variables
					Actor->Serialize(Ar);

					//update the state of actor, calling function so us 'I' prefix
					ISGameplayInterface::Execute_OnActorLoaded(Actor);

					break;
				}
			}
		}

	}
	else
	{
		CurrentSaveGame = Cast<USSaveGame>(UGameplayStatics::CreateSaveGameObject(USSaveGame::StaticClass()));
		UE_LOG(LogTemp, Log, TEXT("Created New SaveGame Data."));
	}
}