// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SAttributeComponent.generated.h"

//create custom events about health change
//thought process: give a delegate name -> current/new health -> how much changed (delta) -> where it changed -> who did it?
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHealthChanged, AActor*, InstigatorActor, USAttributeComponent*, OwningComp, float, NewHealth, float, Delta);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ACTIONROGUELIKE_API USAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USAttributeComponent();

protected:

	//EditAnywhere - edit in BP editor and per_instance in level.
	//VisibleAnywhere - 'read-only' in editor and level. (use for components)
	//EditDefaultsOnly - hide variable per instance, edit in BP editor only
	//EditInstanceOnly - allow only editing instance (eg. when placed in level)
	// - - -
	//BlueprintReadOnly - read-only in the Blueprint scripting (does not affect 'details' panel)
	//BlueprintReadWrite - read-write access in Blueprints
	// - - -
	//Category = "" - display only for detail panels and blueprint context menu

	//setup health variable
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
		float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
		float HealthMax;

	//HealthMax, Stamina, Strength

public:

	//return the living state of a character
	UFUNCTION(BlueprintCallable)
		bool IsAlive() const;

	UFUNCTION(BlueprintCallable)
		bool IsFullHealth() const;

	UFUNCTION(BlueprintCallable)
		float GetHealthMax() const;

	//assignable allows us to bind and subscribe to this event
	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnHealthChanged;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	bool ApplyHealthChange(float Delta);
};
