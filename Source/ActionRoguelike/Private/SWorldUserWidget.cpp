// Fill out your copyright notice in the Description page of Project Settings.


#include "SWorldUserWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/SizeBox.h"
#include "Kismet/GameplayStatics.h"

//check module update

void USWorldUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	//dont want it to spawn if minion is dead, use IsValid() here because it calls PendingKill()
	if (!IsValid(AttachedActor))
	{
		//same as remove from viewport
		RemoveFromParent();
		//debug message to help visualize it
		UE_LOG(LogTemp, Warning, TEXT("AttachedActor no longer valid. Removing health widget"));

		return;
	}

	//never forget Super
	Super::NativeTick(MyGeometry, InDeltaTime);

	FVector2D ScreenPosition;
	//fix of unwanted stuff on the scrren
	bool bIsOnScreen = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(GetOwningPlayer(), AttachedActor->GetActorLocation() + WorldOffset, ScreenPosition, false);

	//every tick, project world location to screen location
	//check if it finds a position on screen
	if(bIsOnScreen)
	{
		//UGameplayStatics::ProjectWorldToScreen //old method
		//we can find this class in BP, it returns the scale of the viewport to our dpi
		//float Scale = UWidgetLayoutLibrary::GetViewportScale(this);

		//find the correct position
		//ScreenPosition /= Scale;

		if(ParentSizeBox)
		{
			//render that box, could be a bar or a text message, on our designed position
			ParentSizeBox->SetRenderTranslation(ScreenPosition);
		}
	}
	if (ParentSizeBox)
	{
		ParentSizeBox->SetVisibility(bIsOnScreen ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}
