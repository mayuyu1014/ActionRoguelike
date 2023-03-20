// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class USInteractionComponent;
class UAnimMontage;
class USAttributeComponent;
class UParticleSystem;
class USActionComponent;

UCLASS()
class ACTIONROGUELIKE_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

protected:

	/* VisibleAnywhere = read-only, still useful to view in-editor and enforce a convention. */
	//the trick here is to make it only for others to see it, but unable to change to avoid potential "bugs"
	UPROPERTY(VisibleAnywhere, Category = "Effects")
	FName TimeToHitParamName;

	//third person camera setup
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArmComp;

	//expose the properties and variables of our component
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* CameraComp;
	//bind interaction behavior with character - e key

	UPROPERTY(VisibleAnywhere)
	USInteractionComponent* InteractionComp;

	//bind attribute component to the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USAttributeComponent* AttributeComp;

	//bind to our own action system
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USActionComponent* ActionComp;

	//movement function
	void MoveForward(float value);
	void MoveRight(float value);

	void SprintStart();
	void SprintStop();

	//attack function - its an action so no need for parameter
	void PrimaryAttack();

	void BlackHoleAttack();

	void Dash();

	//Interact function
	void PrimaryInteract();

	UFUNCTION()
	void OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta);

	//binding events
	virtual void PostInitializeComponents() override;

	//override the original view point function which only suits for FPS
	virtual FVector GetPawnViewLocation() const override;

public:

	// Sets default values for this character's properties
	ASCharacter();

	// Called every frame, used for drawing debug lines
	//virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//cheating codes for debug, can only be added in player character, player controller, game mode, and cheat manager
	UFUNCTION(Exec)
	void HealSelf(float Amount = 100.0f);
};
