// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerController.h"

#include "Blueprint/UserWidget.h"

void ASPlayerController::TogglePauseMenu()
{
	if (PauseMenuInstance && PauseMenuInstance->IsInViewport())
	{
		//if the pause menu is already there, remove it (cancel pause)
		PauseMenuInstance->RemoveFromParent();
		PauseMenuInstance = nullptr;

		//reverse the pause setting
		bShowMouseCursor = false;
		SetInputMode(FInputModeGameOnly());
		//need to return
		return;
	}

	PauseMenuInstance = CreateWidget<UUserWidget>(this, PauseMenuClass);
	if(PauseMenuInstance)
	{
		//a high z-order to ensure its priority
		PauseMenuInstance->AddToViewport(100);

		//show the mouse like in BP
		bShowMouseCursor = true;
		//dont want the inputs to only affect this UI not the actual gameplay
		SetInputMode(FInputModeUIOnly());
	}

}

void ASPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("PauseMenu", IE_Pressed, this, &ASPlayerController::TogglePauseMenu);
}

void ASPlayerController::BeginPlayingState()
{
	BlueprintBeginPlayingState();
}

void ASPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	OnPlayerStateReceived.Broadcast(PlayerState);
}