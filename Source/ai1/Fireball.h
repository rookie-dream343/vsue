// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Fireball.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

/**
 * 火球弹丸 —— 由 BasicMoveCharacter 鼠标左键发射，沿相机方向直线飞行。
 * 击中目标后销毁自身。
 */
UCLASS()
class AI1_API AFireball : public AActor
{
	GENERATED_BODY()

public:
	AFireball();

	/** 设置发射方向和速度 */
	void FireInDirection(const FVector& ShootDirection);

protected:
	// 碰撞球
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fireball")
	TObjectPtr<USphereComponent> CollisionSphere;

	// 弹丸移动组件（自动处理飞行、反弹等）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fireball")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	// 击中时调用
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

	/** 弹丸速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fireball")
	float FireballSpeed = 3000.f;

	/** 存活时间（秒），到期自动销毁 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fireball")
	float Lifetime = 3.f;
};
