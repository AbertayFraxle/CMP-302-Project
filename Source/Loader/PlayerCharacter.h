// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CableComponent.h"
#include "Pylon.h"
#include "Hook.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/BoxComponent.h"
#include "PlayerCharacter.generated.h"

//define damage value that will be a base for all attacks we do
#define DAMAGE 12

//define the cooldowns for the abilities
#define GRAPPLECOOLDOWN 5
#define UTILITYCOOLDOWN 5
#define SPECIALCOOLDOWN 20

//define the limit for dash speed
#define DASHLIMIT 3000
#define DASHRATE 1500

//define attributes for the grappling mechanic
#define GRAPPLERANGE 5000

//define point in attack animation where punch happens and where it loops
#define PDAMAGEPOINT 0.25
#define PRESETPOINT 0.93

//manual gravity value for when the player is swinging
#define MANUALGRAVITY 1000

//forward declare alien to prevent circular dependencies
class AAlien;

UCLASS()
class LOADER_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void damageResponse(int damageValue);


protected:

	//All the variables to be accessed by the HUD object and animation BP, mainly timers for cooldowns
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	bool attacking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	float grappleTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	float utilityTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	float specialTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	float primaryTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	float dashMeter;

	//setup health variables, expose to the HUD
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	int health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	int maxHealth;

	//setup input actions
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UInputAction* moveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputAction* lookAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputAction* jumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputAction* sprintAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputAction* primaryAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputAction* secondaryAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputAction* utilityAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputAction* specialAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputMappingContext* inputMappingContext;

	//set up camera component
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCameraComponent* playerCam;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USpringArmComponent* cameraBoom;

	//set up component for grappling cable
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UCableComponent* grappleCable;

	//set up the spawnable items this character creates
	APylon* currPylon;
	AHook* hook;

	//set up hurt box for damaging enemies
	UBoxComponent* hurtBox;

	//a slot to attach the cable to the left hand of the player
	USceneComponent* handSlot;
	
	//a bool to handle sprinting
	bool sprinting;

	//variables used for the primary attack
	float primaryAttacksPerSecond;
	TArray<AAlien*> overlappingEnemies;
	float hurtTimer;
	bool hasDamaged;

	//variables used in the grapple mechanic
	FVector grapplePoint;
	FVector dirToPlayer;
	bool grappleHit;
	float grappleRange;
	bool grappleGoing;
	float grappleLerp;
	float savedFriction;

	//have to put this here as using it as a #define was throwing a potential division by 0 error
	float GRAPPLESPEED;

	//variables used for the dash ability
	bool dashCharging;
	bool dashing;
	float dashActiveTimer;

	bool attachedToPylon;

	//variable for HUD to feedback to player if point aimed at is reachable with grapple
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool inRange;
	
	//functions for the player ticke event
	void TickDownTimers(float DeltaTime);

	//check if theres a point that can be grappled to
	void TickCheckRange();

	//functionality put in tick event for primary attacks to function
	void TickHandleAttacking(float DeltaTime);

	//handle increasing the dash charge meter before releasing
	void TickHandleDashCharge(float DeltaTime);

	//handle the current dash
	void TickHandleDashCurrent();

	//handle the initial grapple firing
	void TickHandleGrappleCurrent(float DeltaTime);

	//define input action trigger functions

	void Move(const FInputActionValue& value);
	void Look(const FInputActionValue& value);
	void Sprint(const FInputActionValue& value);

	void Primary(const FInputActionValue& value);

	void Secondary(const FInputActionValue& value);
	void SecondaryInProgress(const FInputActionValue& value);
	void SecondaryReleased(const FInputActionValue& value);

	void Utility(const FInputActionValue& value);
	void UtilityRelease(const FInputActionValue& value);

	void Special(const FInputActionValue& values);

	
	//Overlap Begin and End events
	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& hit);

	UFUNCTION()
	void OnBoxEndOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& hit);


};
