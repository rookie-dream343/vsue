// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HelloWorldActor.generated.h"

UCLASS()
class AI1_API AHelloWorldActor : public AActor
{
    GENERATED_BODY()

public:
    AHelloWorldActor();

protected:
    virtual void BeginPlay() override;

private:
    void PrintHelloWorld();

    FTimerHandle HelloTimerHandle;
};
