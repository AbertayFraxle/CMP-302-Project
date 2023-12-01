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

	hook = CreateDefaultSubobject<USceneComponent>(TEXT("Grapple Hook"));

	grappleCable = CreateDefaultSubobject<UCableComponent>(TEXT("Grapple Cable"));
	grappleCable->SetupAttachment(RootComponent);


	cameraBoom->SetupAttachment(RootComponent);

	cameraBoom->bInheritPitch = true;
	cameraBoom->bInheritYaw = true;
	cameraBoom->bUsePawnControlRotation = true;

	playerCam->SetupAttachment(cameraBoom);

	primaryTimer = 0;
	primaryAttacksPerSecond = 2;

	grappleRange = 50000;

	GetCharacterMovement()->MaxWalkSpeed = 700;

	sprinting = false;
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	
	grappleCable->SetVisibility(false);
	hook->SetVisibility(false);

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {
			Subsystem->AddMappingContext(inputMappingContext, 0);
		}
	}

	grappleTimer = 0;
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (primaryTimer > 0)
	{
		primaryTimer -=DeltaTime;
	}

	if (grappleTimer > 0) {
		grappleTimer -= DeltaTime;
	}

	FHitResult hit;

	GetWorld()->LineTraceSingleByChannel(hit, playerCam->GetComponentLocation() + playerCam->GetForwardVector() *100, playerCam->GetComponentLocation() + (playerCam->GetForwardVector() * grappleRange), ECC_Visibility);

	if (hit.GetActor())
	{
		inRange = true;
	}else
	{
		inRange = false;
	}

	if (dashCharging) {
		if (dashMeter < DASHLIMIT) {
			dashMeter += 1000 * DeltaTime;
		}
	}
	else {
		if (dashMeter > 0) {
			GetCharacterMovement()->AddImpulse(dashMeter * playerCam->GetForwardVector(), true);
			dashMeter = 0;
		}
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

		EnhancedInputComponent->BindAction(utilityAction, ETriggerEvent::Started, this, &APlayerCharacter::Utility);
		EnhancedInputComponent->BindAction(utilityAction, ETriggerEvent::Completed, this, &APlayerCharacter::UtilityRelease);


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

	GetWorld()->LineTraceSingleByChannel(hit, playerCam->GetComponentLocation() + playerCam->GetForwardVector() *100, playerCam->GetComponentLocation() + (playerCam->GetForwardVector() * grappleRange), ECC_Visibility);
	

	if (grappleTimer <= 0) {
		if (hit.GetActor()) {
			grappleHit = true;
			grapplePoint = hit.Location;
			FVector diff = hit.Location - GetActorLocation();
			radius = diff.Length();
			hook->SetWorldLocation(grapplePoint);

			grappleCable->SetAttachEndToComponent(hook);

			grappleCable->SetVisibility(true);
			hook->SetVisibility(true);

			if (GetVelocity().Length() <= 0) {
				GetCharacterMovement()->AddImpulse(diff * 1, true);
			}

		}
		else
		{
			grappleHit = false;
		}
	}
}

void APlayerCharacter::SecondaryReleased(const FInputActionValue& value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Secondary Attack Released"));
	//TODO: RELEASE THE SWINGING MECHANIC, KEEP PLAYER'S DIRECTIONAL VELOCITY THE SAME
	if (grappleHit)
	{
		grappleCable->SetVisibility(false);
		hook->SetVisibility(false);

		grappleHit = false;

		grappleTimer = 5.f;
	}
}

void APlayerCharacter::SecondaryInProgress(const FInputActionValue& value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Secondary In Progress"));
	//TODO: IMPLEMENT THE SWINGING
	
	grappleCable->SetWorldLocation(GetActorLocation()-(GetActorRightVector()*20));

	FVector diff =  GetActorLocation() - grapplePoint;

	dirToPlayer =  grapplePoint - playerCam->GetComponentLocation();
	dirToPlayer.Normalize();


	double diffL = diff.Length();
	FVector velo = GetVelocity();

	double dot = diff.Dot(GetVelocity());

	FVector normal = diff / diffL;

	diff.Normalize();
		
	velo = FVector::VectorPlaneProject(velo, normal);
	velo += playerCam->GetForwardVector();

	
	if (dirToPlayer.Dot(playerCam->GetForwardVector()) >=0.99) {

		float strength = velo.Length();
		
		velo = (strength) * -diff;
		
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("MOVING TOWARDS"));
	}
	

	if (dot >= 0  && grappleHit) {
		GetCharacterMovement()->Velocity = velo;
	}

	
	
}


void APlayerCharacter::Utility(const FInputActionValue& value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Utility Abilty Used"));
	//TODO: Add loader's dash attack

	dashCharging = true;

}

void APlayerCharacter::UtilityRelease(const FInputActionValue& value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Utility Abilty Released"));

	dashCharging = false;
}

void APlayerCharacter::Special(const FInputActionValue& value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Special Ability Used"));
	//TODO: Add the pylon to do damage and be grappled to
	currPylon = GetWorld()->SpawnActor<APylon>(GetActorLocation(),GetActorRotation());
}

void APlayerCharacter::Sprint(const FInputActionValue& value)
{
	
	if (!sprinting)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Sprinting is true"));
		GetCharacterMovement()->MaxWalkSpeed = 700 * 1.45;
		sprinting = true;
	}else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Sprinting is false"));
		sprinting = false;
		GetCharacterMovement()->MaxWalkSpeed = 700;
	}
}

