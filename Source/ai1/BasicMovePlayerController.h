// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BasicMovePlayerController.generated.h"

class UInputMappingContext;

/**
 * 为 Enhanced Input 鼠标视角设计的 PlayerController。
 * 提供 AddInputMappingContext 供 Pawn 的 BeginPlay 调用。
 */
UCLASS()
class AI1_API ABasicMovePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** 由 Pawn 调用，注册其动态创建的 IMC */
	void AddInputMappingContext(UInputMappingContext* IMC, int32 Priority = 0);
};
