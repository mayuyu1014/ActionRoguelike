// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameModeBase.h"

#include "EngineUtils.h"
#include "SAttributeComponent.h"
#include "AI/SAICharacter.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "DrawDebugHelpers.h"
#include "SCharacter.h"

//console variables for debug, ECVF_Cheat marks this function to not be included in the final build
static TAutoConsoleVariable<bool>CVarSpawnBots(TEXT("su.SpawnBots"), true, TEXT("Enable spawning of bots via timer."), ECVF_Cheat);


ASGameModeBase::ASGameModeBase()
{
	SpawnTimerInterval = 2.0f;
}

void ASGameModeBase::StartPlay()
{
	//dont forget this
	Super::StartPlay();

	//setup a timer to call the spawn query, and find a spawn spot
	GetWorldTimerManager().SetTimer(TimerHandle_SpawnBots, this, &ASGameModeBase::SpawnBotTimerElapsed, SpawnTimerInterval, true);
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
		QueryInstance->GetOnQueryFinishedEvent().AddDynamic(this, &ASGameModeBase::OnQueryCompleted);
	}
}

void ASGameModeBase::OnQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus)
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
		//if we at least have 1 winner result from EQS query, spawn a bot
		//to assign a specific MinionClass, we do it in BP
		GetWorld()->SpawnActor<AActor>(MinionClass, Locations[0], FRotator::ZeroRotator);

		//track all the used spawn locations
		DrawDebugSphere(GetWorld(), Locations[0], 50.0f, 20, FColor::Blue, false, 60.0f);
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

	//some debug message
	UE_LOG(LogTemp, Log, TEXT("OnActorKilled: Victim: %s, Killer: %s"), *GetNameSafe(VictimActor), *GetNameSafe(Killer));
}