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

	//a static getter to make things easier to get something from this attribute class
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	static USAttributeComponent* GetAttributes(AActor* FromActor);

	//another static for easy access to check if AActor IsAlive()
	UFUNCTION(BlueprintCallable, Category = "Attributes", meta = (DisplayName = "IsAlive"))
	static bool IsActorAlive(AActor* Actor);

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

	//might be useful in some cases
	UFUNCTION(BlueprintCallable)
	bool Kill(AActor* InstigatorActor);

	//return the living state of a character
	UFUNCTION(BlueprintCallable)
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable)
	bool IsFullHealth() const;

	UFUNCTION(BlueprintCallable)
	float GetHealth() const;

	UFUNCTION(BlueprintCallable)
	float GetHealthMax() const;

	//assignable allows us to bind and subscribe to this event
	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnHealthChanged;

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	bool ApplyHealthChange(AActor* InstigatorActor, float Delta);
};
