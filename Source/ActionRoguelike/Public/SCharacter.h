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

UCLASS()
class ACTIONROGUELIKE_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

protected:

	/* VisibleAnywhere = read-only, still useful to view in-editor and enforce a convention. */
	//the trick here is to make it only for others to see it, but unable to change to avoid potential "bugs"
	UPROPERTY(VisibleAnywhere, Category = "Effects")
		FName TimeToHitParamName;

	UPROPERTY(VisibleAnywhere, Category = "Effects")
		FName HandSocketName;

	//assign our projectile class here, needs to be added in blueprint
	//using category to be more orgainized, and easy to find in blueprint
	UPROPERTY(EditAnywhere, Category = "Attack")
		TSubclassOf<AActor> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Attack")
		TSubclassOf<AActor> BlackHoleProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Attack")
		TSubclassOf<AActor> DashProjectileClass;

	//declare a pointer pointing to this existing attack animation
	UPROPERTY(EditAnywhere, Category = "Attack")
	UAnimMontage* AttackAnim;

	/* Particle System played during attack animation */
	UPROPERTY(EditAnywhere, Category = "Attack")
		UParticleSystem* CastingEffect;

	//declare to create the Timer Handler
	FTimerHandle TimerHandle_PrimaryAttack;
	FTimerHandle TimerHandle_BlackholeAttack;
	FTimerHandle TimerHandle_Dash;

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
		float AttackAnimDelay;

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

	//movement function
	void MoveForward(float value);
	void MoveRight(float value);

	//attack function - its an action so no need for parameter
	void PrimaryAttack();
	//Timer elapsed for primary attack
	void PrimaryAttack_Elapsed();

	void BlackHoleAttack();
	void BlackholeAttack_TimeElapsed();

	void Dash();
	void Dash_TimeElapsed();

	//casting effects
	void StartAttackEffects();

	// Re-use spawn logic between attacks
	void SpawnProjectile(TSubclassOf<AActor> ClassToSpawn);

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
