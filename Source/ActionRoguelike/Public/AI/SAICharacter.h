// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SAICharacter.generated.h"

class UPawnSensingComponent;
class USAttributeComponent;
class UUserWidget;
class USWorldUserWidget;

UCLASS()
class ACTIONROGUELIKE_API ASAICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASAICharacter();

protected:

	//for internal use to track if the minion already has a health bar or not
	USWorldUserWidget* ActiveHealthBar;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> HealthBarWidgetClass;

	//read only
	UPROPERTY(VisibleAnywhere, Category = "Effects")
	FName TimeToHitParamName;

	//for internal use
	void SetTargetActor(AActor* NewTarget);

	UFUNCTION()
	void OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta);

	//inherited from parent class, necessary to have to implement functions from parent class
	virtual void PostInitializeComponents() override;

	//give AI some basic senses
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UPawnSensingComponent* PawnSensingComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USAttributeComponent* AttributeComp;

	//delegate function from parent class - UPawnSensingComponent
	UFUNCTION()
	void OnPawnSeen(APawn* Pawn);
};
