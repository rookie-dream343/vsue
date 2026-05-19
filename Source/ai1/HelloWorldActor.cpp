// Fill out your copyright notice in the Description page of Project Settings.

#include "HelloWorldActor.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

AHelloWorldActor::AHelloWorldActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AHelloWorldActor::BeginPlay()
{
    Super::BeginPlay();

    if (GetWorld())
    {
        GetWorldTimerManager().SetTimer(HelloTimerHandle, this, &AHelloWorldActor::PrintHelloWorld, 1.0f, true);
    }
}

void AHelloWorldActor::PrintHelloWorld()
{
    UE_LOG(LogTemp, Log, TEXT("hello world"));
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("hello world"));
    }
}
