// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SWorldUserWidget.generated.h"

class USizeBox;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API USWorldUserWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	//setup a parent widget for every child widget to bind to inherit the functions in this class
	UPROPERTY(meta = (BindWidget))
	USizeBox* ParentSizeBox;

	//inherit from parent class
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:

	//offset the location of this widget
	UPROPERTY(EditAnywhere, Category = "UI")
	FVector WorldOffset;

	//broaddast this to UPROPERTY system for safe handling of the pointers
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	AActor* AttachedActor;
};
