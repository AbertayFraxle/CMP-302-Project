// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/BoxComponent.h"
#include "PlayerCharacter.h"
#include "Alien.generated.h"

//these are used to determine the point in the animation clip at when the damage should be applied to the player
#define DAMAGEPOINT 1.11
#define RESETPOINT 2.63
#define ADAMAGE 12

UCLASS()
class LOADER_API AAlien : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AAlien();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool attacking;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//store the player to find direction to move and also for attacking purposes
	APawn* player;
	
	//hurtbox to check if the player is in melee range
	UBoxComponent* hurtBox;

	//timer variable to hurt the player at correct point of attack
	float hurtTimer;
	bool hasHit;
	bool playerOverlap;

	//variables for knockback
	bool knocked;
	float knockTimer;

	//timer to despawn corpse once dead
	float rotTimer;

	//enemy health and dead condition, dead bool exposed so animation blueprint can change appropriately
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	bool dead; 
	int health;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//getter and setter for knockback
	bool getKnocked();
	void setKnocked(bool nKnock);

	//reduce the health
	void reduceHealth(int reduction);

	//enter/exit hurtbox functions
	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& hit);

	UFUNCTION()
	void OnBoxEndOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& hit);
	
};
