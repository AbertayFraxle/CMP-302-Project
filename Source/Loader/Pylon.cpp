// Fill out your copyright notice in the Description page of Project Settings.


#include "Pylon.h"

// Sets default values
APylon::APylon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APylon::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APylon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

