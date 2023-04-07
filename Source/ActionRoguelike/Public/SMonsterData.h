// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SMonsterData.generated.h"

class USAction;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API USMonsterData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	//specify the monster class to spawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Info")
	TSubclassOf<AActor> MonsterClass;

	//Actions/buffs to grant this monster
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Info")
	TArray<TSubclassOf<USAction>> Actions;

	//specify the icon, if we need it in UI
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	UTexture2D* Icon;

	FPrimaryAssetId GetPrimaryAssetId() const override
	{
		//get FName of the data asset, first param is actually spell sensitive
		return FPrimaryAssetId("Monsters", GetFName());
	}
};
