// Fill out your copyright notice in the Description page of Project Settings.

#include "BasicMovePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"

void ABasicMovePlayerController::AddInputMappingContext(UInputMappingContext* IMC, int32 Priority)
{
	if (!IMC) return;

	UEnhancedInputLocalPlayerSubsystem* Sub = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Sub && !Sub->HasMappingContext(IMC))
	{
		Sub->AddMappingContext(IMC, Priority);
	}
}
