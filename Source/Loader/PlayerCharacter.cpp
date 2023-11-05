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

	primaryTimer = 0;
	primaryAttacksPerSecond = 2;

	grappleRange = 50000;

	sprinting = false;
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

	if (primaryTimer > 0)
	{
		primaryTimer -=DeltaTime;
	}

	FHitResult hit;

	GetWorld()->LineTraceSingleByChannel(hit, GetActorLocation()+GetActorForwardVector()*100, playerCam->GetComponentLocation() + (playerCam->GetForwardVector() * grappleRange), ECC_Visibility);

	if (hit.GetActor())
	{
		inRange = true;
	}else
	{
		inRange = false;
	}
	
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		EnhancedInputComponent->BindAction(moveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);

		EnhancedInputComponent->BindAction(lookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);

		EnhancedInputComponent->BindAction(jumpAction, ETriggerEvent::Started, this, &Super::Jump);

		EnhancedInputComponent->BindAction(sprintAction, ETriggerEvent::Started, this, &APlayerCharacter::Sprint);
		
		EnhancedInputComponent->BindAction(primaryAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Primary);

		EnhancedInputComponent->BindAction(secondaryAction, ETriggerEvent::Started, this, &APlayerCharacter::Secondary);
		EnhancedInputComponent->BindAction(secondaryAction, ETriggerEvent::Ongoing, this, &APlayerCharacter::SecondaryInProgress);
		EnhancedInputComponent->BindAction(secondaryAction, ETriggerEvent::Completed, this, &APlayerCharacter::SecondaryReleased);

		EnhancedInputComponent->BindAction(utilityAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Utility);

		EnhancedInputComponent->BindAction(specialAction, ETriggerEvent::Started, this, &APlayerCharacter::Special);

	}

}

void APlayerCharacter::Move(const FInputActionValue& value)
{
	FVector2D MovementVector = value.Get<FVector2D>();
	float modifier = 0.69;

	if (sprinting)
	{
		modifier = 1.f;
	}
	
	AddMovementInput(GetActorForwardVector(), MovementVector.Y*modifier);
	AddMovementInput(GetActorRightVector(), MovementVector.X*modifier);
}

void APlayerCharacter::Look(const FInputActionValue& value)
{
	FVector2D LookVector = value.Get<FVector2D>();

	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);
	
}

void APlayerCharacter::Primary(const FInputActionValue& value)
{
	if (primaryTimer <=0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Primary Attack Done"));
		primaryTimer = 1.f/primaryAttacksPerSecond;
	}
	//TODO: Implement Loader's default melee attack
}

void APlayerCharacter::Secondary(const FInputActionValue& value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Secondary Attack Done"));

	//TODO: IMPLEMENT THE SWINGING Anchor point
	FHitResult hit;

	GetWorld()->LineTraceSingleByChannel(hit, GetActorLocation()+GetActorForwardVector()*100, playerCam->GetComponentLocation() + (playerCam->GetForwardVector() * grappleRange), ECC_Visibility);
	DrawDebugLine(GetWorld(), GetActorLocation() + GetActorForwardVector() * 100, playerCam->GetComponentLocation() + (playerCam->GetForwardVector() * grappleRange), FColor::Red,false,5.f);

	if (hit.GetActor()) {
		grappleHit = true;
		grapplePoint = hit.Location;
		FVector diff = hit.Location - GetActorLocation();
		radius = diff.Length();
	}else
	{
		grappleHit = false;
	}

}

void APlayerCharacter::SecondaryReleased(const FInputActionValue& value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Secondary Attack Released"));
	//TODO: RELEASE THE SWINGING MECHANIC, KEEP PLAYER'S DIRECTIONAL VELOCITY THE SAME
	if (grappleHit)
	{
		
	}
}

void APlayerCharacter::SecondaryInProgress(const FInputActionValue& value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Secondary In Progress"));
	//TODO: IMPLEMENT THE SWINGING
	
	
}


void APlayerCharacter::Utility(const FInputActionValue& value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Utility Abilty Used"));
	//TODO: Add loader's dash attack
}

void APlayerCharacter::Special(const FInputActionValue& value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Special Ability Used"));
	//TODO: Add the pylon to do damage and be grappled to
}

void APlayerCharacter::Sprint(const FInputActionValue& value)
{
	
	if (!sprinting)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Sprinting is true"));
		sprinting = true;
	}else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Sprinting is false"));
		sprinting = false;
	}
}

