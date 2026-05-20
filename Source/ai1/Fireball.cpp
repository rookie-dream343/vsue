// Fill out your copyright notice in the Description page of Project Settings.

#include "Fireball.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AFireball::AFireball()
{
	PrimaryActorTick.bCanEverTick = false;

	// 碰撞球（根组件）
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(20.f);
	CollisionSphere->SetCollisionProfileName(TEXT("Projectile"));
	CollisionSphere->SetNotifyRigidBodyCollision(true);
	CollisionSphere->OnComponentHit.AddDynamic(this, &AFireball::OnHit);
	RootComponent = CollisionSphere;

	// 弹丸移动组件
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->InitialSpeed = FireballSpeed;
	ProjectileMovement->MaxSpeed = FireballSpeed;
	ProjectileMovement->ProjectileGravityScale = 0.f; // 无重力，直线飞行

	// 默认生命周期
	InitialLifeSpan = Lifetime;
}

void AFireball::FireInDirection(const FVector& ShootDirection)
{
	ProjectileMovement->Velocity = ShootDirection * FireballSpeed;
}

void AFireball::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	// 击中后销毁（后续可扩展伤害逻辑）
	if (OtherActor && OtherActor != this)
	{
		Destroy();
	}
}
