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
#include "PlayerCharacter.generated.h"

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

protected:

	//setup input actions
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UInputAction* moveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputAction* lookAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInputAction* jumpAction;

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

	//set up variables 
	FVector grapplePoint;
	bool grappleHit;

	float primaryTimer;
	float primaryAttacksPerSecond;

	//define input action trigger functions

	void Move(const FInputActionValue& value);
	void Look(const FInputActionValue& value);
	
	void Primary(const FInputActionValue& value);

	void Secondary(const FInputActionValue& value);
	void SecondaryInProgress(const FInputActionValue& value);
	void SecondaryReleased(const FInputActionValue& value);

	void Utility(const FInputActionValue& value);
	void Special(const FInputActionValue& values);



};
