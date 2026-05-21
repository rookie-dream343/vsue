// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Fireball.generated.h"

class USphereComponent;

/**
 * 火球弹丸 —— 左键/E键发射，沿相机方向直线飞行。
 * 击中物体或 3 秒后自毁。
 */
UCLASS()
class AI1_API AFireball : public AActor
{
	GENERATED_BODY()

public:
	AFireball();
	virtual void Tick(float DeltaTime) override;

	/** 设置飞行方向和速度 */
	void FireInDirection(const FVector& ShootDirection);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshSphere;

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	FVector Velocity;

	UPROPERTY(EditAnywhere, Category = "Fireball")
	float Speed = 4000.f;

	UPROPERTY(EditAnywhere, Category = "Fireball")
	float LifeTime = 3.f;

	float Elapsed = 0.f;
};
