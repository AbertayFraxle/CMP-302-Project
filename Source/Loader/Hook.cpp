// Fill out your copyright notice in the Description page of Project Settings.


#include "Hook.h"

// Sets default values
AHook::AHook()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//load the hook model and set this as the root component
	model = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Model"));
	const ConstructorHelpers::FObjectFinder<UStaticMesh> mesh(TEXT("/Script/Engine.StaticMesh'/Game/Models/fist.fist'"));
	model->SetStaticMesh(mesh.Object);
	model->SetWorldRotation(FQuat(FVector(1,0,0),-3.14/2));
	model->SetWorldScale3D(FVector(40,40,40));

	SetRootComponent(model);
	
}

// Called when the game starts or when spawned
void AHook::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHook::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

