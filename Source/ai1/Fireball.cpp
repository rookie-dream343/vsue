// Fill out your copyright notice in the Description page of Project Settings.

#include "Fireball.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AFireball::AFireball()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(20.f);
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionSphere->SetGenerateOverlapEvents(true);
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AFireball::OnOverlap);
	RootComponent = CollisionSphere;

	MeshSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshSphere"));
	MeshSphere->SetupAttachment(CollisionSphere);
	MeshSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshSphere->SetRelativeScale3D(FVector(0.4f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		MeshSphere->SetStaticMesh(SphereMesh.Object);
	}
}

void AFireball::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Elapsed += DeltaTime;
	if (Elapsed > LifeTime)
	{
		Destroy();
		return;
	}

	// 每帧自己移动
	FVector Delta = Velocity * DeltaTime;
	AddActorWorldOffset(Delta, true);
}

void AFireball::FireInDirection(const FVector& ShootDirection)
{
	Velocity = ShootDirection.GetSafeNormal() * Speed;
}

void AFireball::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && OtherActor != GetOwner() && OtherActor != GetInstigator())
	{
		Destroy();
	}
}
