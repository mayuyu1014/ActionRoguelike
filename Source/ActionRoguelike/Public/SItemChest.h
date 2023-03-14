// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SGameplayInterface.h"
#include "SItemChest.generated.h"

class UStaticMeshComponent;

UCLASS()
class ACTIONROGUELIKE_API ASItemChest : public AActor, public ISGameplayInterface
{
	GENERATED_BODY()

public:
	//pitch value of the chest lid
	UPROPERTY(EditAnywhere)
	float TargetPitch;
	//after inherit the gameplay interface, need to implement it here
	//_Implementation is needed here because we specified it as a BlueprintNativeEvent
	void Interact_Implementation(APawn* InstigatorPawn);
	
public:	
	// Sets default values for this actor's properties
	ASItemChest();

protected:
	//create two functions for this chest, a base + a lid
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* BaseMesh;
	//add BlueprintReadOnly to animate the lid opening in blueprint
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UStaticMeshComponent* LidMesh;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
