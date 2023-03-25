// Fill out your copyright notice in the Description page of Project Settings.


#include "SAction.h"
#include "SActionComponent.h"
#include "ActionRoguelike/ActionRoguelike.h"
#include "Net/UnrealNetwork.h"

bool USAction::CanStart_Implementation(AActor* Instigator)
{
	//dont start when it is already started
	if(IsRunning())
	{
		return false;
	}

	//check against blocked attacks
	USActionComponent* Comp = GetOwningComponent();
	if (Comp->ActiveGameplayTags.HasAny(BlockedTags))
	{
		return false;
	}
	return true;
}

//override in blueprint
void USAction::StartAction_Implementation(AActor* Instigator)
{
	//debugging message
	UE_LOG(LogTemp, Log, TEXT("Running %s"), *GetNameSafe(this));
	//LogOnScreen(this, FString::Printf(TEXT("Started: %s"), *ActionName.ToString()), FColor::Green);

	USActionComponent* Comp = GetOwningComponent();

	//append tag like its c_str, not add
	Comp->ActiveGameplayTags.AppendTags(GrantsTags);

	RepData.bIsRunning = true;
	RepData.Instigator = Instigator;
}
//override in blueprint
void USAction::StopAction_Implementation(AActor* Instigator)
{
	//debugging message
	UE_LOG(LogTemp, Log, TEXT("Stopped %s"), *GetNameSafe(this));
	//LogOnScreen(this, FString::Printf(TEXT("Started: %s"), *ActionName.ToString()), FColor::Red);

	//always check if the boolean is set, not suitable in multiplayer cuz client cant check it
	//ensureAlways(IsRunning());

	USActionComponent* Comp = GetOwningComponent();
	//append tag like its c_str, not add
	Comp->ActiveGameplayTags.RemoveTags(GrantsTags);

	RepData.bIsRunning = false;
	RepData.Instigator = Instigator;
}

UWorld* USAction::GetWorld() const
{
	//Outer is set when creating action via NewObject<T>
	//by default, the GetWorld() in parent class is not properly implemented that prevents some functions to be called in BP
	//this is a special implicit access to GetWorld()
	UActorComponent* Comp = Cast<UActorComponent>(GetOuter());
	if (Comp)
	{
		return Comp->GetWorld();
	}
	return nullptr;
}

USActionComponent* USAction::GetOwningComponent() const
{
	return Cast<USActionComponent>(GetOuter());
}

void USAction::OnRep_RepData()
{
	if(RepData.bIsRunning)
	{
		StartAction(RepData.Instigator);
	}else
	{
		StopAction(RepData.Instigator);
	}
}

bool USAction::IsRunning() const
{
	return RepData.bIsRunning;
}

void USAction::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USAction, RepData);
}