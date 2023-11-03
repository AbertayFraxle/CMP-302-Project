// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	playerCam = CreateDefaultSubobject<UCameraComponent>(TEXT("Player Camera"));

	cameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	cameraBoom->TargetArmLength = 500;

	cameraBoom->SetupAttachment(RootComponent);

	cameraBoom->bInheritPitch = true;
	cameraBoom->bInheritYaw = true;
	cameraBoom->bUsePawnControlRotation = true;

	playerCam->SetupAttachment(cameraBoom);
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {
			Subsystem->AddMappingContext(inputMappingContext, 0);
		}
	}

}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		EnhancedInputComponent->BindAction(moveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);

		EnhancedInputComponent->BindAction(lookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);

		EnhancedInputComponent->BindAction(jumpAction, ETriggerEvent::Started, this, &APlayerCharacter::Jump);

		EnhancedInputComponent->BindAction(primaryAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Primary);

		EnhancedInputComponent->BindAction(secondaryAction, ETriggerEvent::Started, this, &APlayerCharacter::Secondary);
		EnhancedInputComponent->BindAction(secondaryAction, ETriggerEvent::Completed, this, &APlayerCharacter::SecondaryReleased);

		EnhancedInputComponent->BindAction(utilityAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Utility);

		EnhancedInputComponent->BindAction(specialAction, ETriggerEvent::Started, this, &APlayerCharacter::Special);

	}

}

void APlayerCharacter::Move(const FInputActionValue& value)
{
	FVector2D MovementVector = value.Get<FVector2D>();

	AddMovementInput(GetActorForwardVector(), MovementVector.Y);
	AddMovementInput(GetActorRightVector(), MovementVector.X);
}

void APlayerCharacter::Look(const FInputActionValue& value)
{
	FVector2D LookVector = value.Get<FVector2D>();

	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);

}

void APlayerCharacter::Jump(const FInputActionValue& value) {
	Super::Jump();
}

void APlayerCharacter::Primary(const FInputActionValue& value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Primary Attack Done"));
}

void APlayerCharacter::Secondary(const FInputActionValue& value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Secondary Attack Done"));
}

void APlayerCharacter::SecondaryReleased(const FInputActionValue& value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Secondary Attack Released"));
}

void APlayerCharacter::Utility(const FInputActionValue& value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Utility Abilty Used"));
}

void APlayerCharacter::Special(const FInputActionValue& value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Special Ability Used"));
}

