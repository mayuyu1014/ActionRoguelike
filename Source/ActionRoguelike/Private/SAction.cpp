// Fill out your copyright notice in the Description page of Project Settings.


#include "SAction.h"
#include "SActionComponent.h"

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

	USActionComponent* Comp = GetOwningComponent();

	//append tag like its c_str, not add
	Comp->ActiveGameplayTags.AppendTags(GrantsTags);

	bIsRunning = true;
}
//override in blueprint
void USAction::StopAction_Implementation(AActor* Instigator)
{
	//debugging message
	UE_LOG(LogTemp, Log, TEXT("Stopped %s"), *GetNameSafe(this));

	//always check if the boolean is set
	ensureAlways(IsRunning());

	USActionComponent* Comp = GetOwningComponent();
	//append tag like its c_str, not add
	Comp->ActiveGameplayTags.RemoveTags(GrantsTags);

	bIsRunning = false;
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

bool USAction::IsRunning() const
{
	return bIsRunning;
}
