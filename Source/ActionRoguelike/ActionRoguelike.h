// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

//debug helper
static void LogOnScreen(UObject* WorldConstext, FString Msg, FColor Color=FColor::White, float Duration=5.0f)
{
	if (!ensure(WorldConstext))
	{
		return;
	}

	UWorld* World = WorldConstext->GetWorld();
	if(!ensure(World))
	{
		return;
	}

	FString NetPrefix = World->IsNetMode(NM_Client) ? "[Client]" : "[Server]";
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, Duration, Color, NetPrefix + Msg);
	}
}