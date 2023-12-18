// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Pylon.generated.h"

#define ZAPDAMAGE 12
#define SHOTS 14
#define DEPLOYSPEED 800

class AAlien;

UCLASS()
class LOADER_API APylon : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	APylon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//model for the pylon
	UStaticMeshComponent* model;

	//the collision sphere for hitting the enemies
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USphereComponent* hurtSphere;

	//angle for rotation
	float angle;

	//counter to kill pylon after it shoots the alloted amount
	int counter;

	//timer for attack cooldown and for deployment
	float zapTimer;
	float deployTimer;

	//direction to move during deployment
	FVector dir;

	//FX system to show the beams hitting the enemies
	UNiagaraSystem* beamTemp;

	//array to store the overlapping enemies
	TArray<AAlien*> overlappingEnemies;

	//collision sphere entry/exit events
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& hit);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& hit);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//set the direction to move during deployment
	void setDir(FVector nDir);


	

};
