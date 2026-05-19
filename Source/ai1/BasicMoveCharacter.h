// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BasicMoveCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * 完整的第三人称角色：
 * - WASD 地面移动 + 鼠标视角旋转
 * - 空格跳跃 + 多段跳支持
 * - 左 Shift 按住飞行，松开后受重力坠落
 * - 全部输入通过 Enhanced Input 系统 + 动态创建 InputAction / IMC
 */
UCLASS()
class AI1_API ABasicMoveCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABasicMoveCharacter();
	virtual void Tick(float DeltaTime) override;

protected:
	// --- 生命周期 ---
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// --- 输入初始化 ---
	void InitializeInputAssets();

	// --- 输入回调 ---
	void Input_MoveForward(const FInputActionValue& Value);
	void Input_MoveRight(const FInputActionValue& Value);
	void Input_LookYaw(const FInputActionValue& Value);
	void Input_LookPitch(const FInputActionValue& Value);
	void Input_JumpPressed();
	void Input_JumpReleased();
	void Input_FlyPressed();
	void Input_FlyReleased();

	// --- 飞行 ---
	void MaintainFlight(float DeltaTime);

protected:
	// ==================== 组件 ====================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class UCameraComponent> FollowCamera;

	// ==================== 输入资产（动态创建）====================

	UPROPERTY()
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY()
	TObjectPtr<UInputAction> MoveForwardAction;

	UPROPERTY()
	TObjectPtr<UInputAction> MoveRightAction;

	UPROPERTY()
	TObjectPtr<UInputAction> LookYawAction;

	UPROPERTY()
	TObjectPtr<UInputAction> LookPitchAction;

	UPROPERTY()
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY()
	TObjectPtr<UInputAction> FlyAction;

	// ==================== 状态 ====================

	bool bInputAssetsInitialized = false;
	bool bIsFlying = false;

	/** 鼠标视角输入累积（Input_Look 写入，Tick 消费并清零） */
	FVector2D PendingLookDelta = FVector2D::ZeroVector;

	// ==================== 飞行参数 ====================

	/** 飞行最大速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
	float FlyingMaxSpeed = 2000.f;

	/** 飞行速度平滑度（越高响应越快） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
	float FlyingInterpSpeed = 8.f;
};
