// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"

#include "VectorTypes.h"
#include "Alien.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;



	//create camera for player to use
	playerCam = CreateDefaultSubobject<UCameraComponent>(TEXT("Player Camera"));

	//create the spring arm component to attach to the camera for third person camera
	cameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	cameraBoom->TargetArmLength = 500;

	//attach camera arm to root component, set variables to be usable with CharacterMovementController
	cameraBoom->SetupAttachment(RootComponent);
	cameraBoom->bInheritPitch = true;
	cameraBoom->bInheritYaw = true;
	cameraBoom->bUsePawnControlRotation = true;

	//setup attachment for camera to end of arm
	playerCam->SetupAttachment(cameraBoom);

	//setup component on created slot in skeletal mesh for cable to come out of and for hook to sit on when not grappling
	handSlot = CreateDefaultSubobject<USceneComponent>(TEXT("slot"));
	handSlot->SetupAttachment(FindComponentByClass<USkeletalMeshComponent>(),TEXT("handSocket"));
	
	//create the cable component to show grapple attachment
	grappleCable = CreateDefaultSubobject<UCableComponent>(TEXT("Grapple Cable"));
	grappleCable->CableWidth = 10;
	grappleCable->bEnableCollision = false;
	grappleCable->bAttachStart= true;
	grappleCable->NumSegments = 1;

	GRAPPLESPEED = 5;

	//create delegates for hurtbox entry/exit functions
	FScriptDelegate beginOverlapDelegate;
	beginOverlapDelegate.BindUFunction(this, "OnBoxBeginOverlap");

	FScriptDelegate endOverlapDelegate;
	endOverlapDelegate.BindUFunction(this, "OnBoxEndOverlap");
	
	//setup hurtbox bounds and attachment
	hurtBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Hurtbox"));
	hurtBox->SetBoxExtent(FVector(50, 50, 50));
	hurtBox->AddRelativeLocation(FVector(50, 0, 0));
	hurtBox->SetupAttachment(RootComponent);
	hurtBox->SetGenerateOverlapEvents(true);

	//add delegates to overlap events to call functions on entry/exit by enemies
	hurtBox->OnComponentBeginOverlap.AddUnique(beginOverlapDelegate);
	hurtBox->OnComponentEndOverlap.AddUnique(endOverlapDelegate);
	hurtBox->SetActive(false);

	//initialise variables to allow primary attacking
	primaryTimer = 0;
	primaryAttacksPerSecond = 2;

	//initialise the max health of the player
	maxHealth = 160;
	health = maxHealth;

	//correctly tag this actor so can be hit by enemies
	Tags.Add("Player");

	//set the maximum walk speed of the player and initialise sprinting
	GetCharacterMovement()->MaxWalkSpeed = 700;
	sprinting = false;

	//load the player's skeletal mesh and the extended anim
	const ConstructorHelpers::FObjectFinder<USkeletalMesh> mesh(TEXT("/Script/Engine.SkeletalMesh'/Game/UsedAnims/SKM_Manny.SKM_Manny'"));
	FindComponentByClass<USkeletalMeshComponent>()->SetSkeletalMesh(mesh.Object);

	const ConstructorHelpers::FObjectFinder<UAnimBlueprint> abp(TEXT("/Script/Engine.AnimBlueprint'/Game/Blueprints/ABP_Manny.ABP_Manny'"));
	FindComponentByClass<USkeletalMeshComponent>()->SetAnimInstanceClass(abp.Object->GeneratedClass);

	//load all of the input actions and the input mapping context
	moveAction = ConstructorHelpers::FObjectFinder<UInputAction>(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/IA_Move.IA_Move'")).Object;
	lookAction = ConstructorHelpers::FObjectFinder<UInputAction>(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/IA_Look.IA_Look'")).Object;
	jumpAction = ConstructorHelpers::FObjectFinder<UInputAction>(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/IA_Jump.IA_Jump'")).Object;
	sprintAction = ConstructorHelpers::FObjectFinder<UInputAction>(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/IA_Sprint.IA_Sprint'")).Object;
	primaryAction = ConstructorHelpers::FObjectFinder<UInputAction>(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/IA_Primary.IA_Primary'")).Object;
	secondaryAction = ConstructorHelpers::FObjectFinder<UInputAction>(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/IA_Secondary.IA_Secondary'")).Object;
	utilityAction = ConstructorHelpers::FObjectFinder<UInputAction>(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/IA_Utility.IA_Utility'")).Object;
	specialAction = ConstructorHelpers::FObjectFinder<UInputAction>(TEXT("/Script/EnhancedInput.InputAction'/Game/Input/IA_Special.IA_Special'")).Object;
	inputMappingContext = ConstructorHelpers::FObjectFinder<UInputMappingContext>(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/Input/IMC_Player.IMC_Player'")).Object;


}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	//spawn a hook object in the world, and set cable to be invisible
	hook = GetWorld()->SpawnActor<AHook>(GetActorLocation()+(GetActorForwardVector()*100),GetActorRotation());
	grappleCable->SetVisibility(false);


	//get the player's controller and set up Enhanced Input mapping context using it
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {
			Subsystem->AddMappingContext(inputMappingContext, 0);
		}
	}

	//initialise the grappleTimer
	grappleTimer = 0;
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//tick down all timers that need to be reduced
	TickDownTimers(DeltaTime);

	//check if the player cna grapple to anything. to be used by HUD
	TickCheckRange();

	//handle the attacking functionality in the tick event
	TickHandleAttacking(DeltaTime);

	//charge the dash meter and add impulse when released
	TickHandleDashCharge(DeltaTime);

	//handle the current dash happening
	TickHandleDashCurrent();
	
	//handle the beginning and end grapple functionality
	TickHandleGrappleCurrent(DeltaTime);
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//bind all functions to the input actions
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

void APlayerCharacter::damageResponse(int damageValue)
{
	//reduce health, and if its below 0 
	if (health > 0) {
		health -= damageValue;
		if (health < 0) {
			health = 0;
		}
	}
}

void APlayerCharacter::Move(const FInputActionValue& value)
{
	FVector2D MovementVector = value.Get<FVector2D>();
	MovementVector.Normalize();
	//if not dashing, add the input vector to the movement component to move player
	if (!dashing) {
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void APlayerCharacter::Look(const FInputActionValue& value)
{
	FVector2D LookVector = value.Get<FVector2D>();

	//use vector from mouse input to the camera
	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);
	
}

void APlayerCharacter::Primary(const FInputActionValue& value)
{
	//if set state to attacking if not on cooldown, then set cooldown
	if (primaryTimer <= 0)
	{
		attacking = true;
		primaryTimer = 1.f / primaryAttacksPerSecond;
	}
}

void APlayerCharacter::Secondary(const FInputActionValue& value)
{

	//when secondary is fired do raycast to point, get if it hit and get the location of the hit point
	FHitResult hit;
	GetWorld()->LineTraceSingleByChannel(hit, playerCam->GetComponentLocation() + (playerCam->GetForwardVector() *100), playerCam->GetComponentLocation() + (playerCam->GetForwardVector() * GRAPPLERANGE), ECC_Visibility);
	
	//if the cooldown is over
	if (grappleTimer <= 0) {

		//set grapple state to going to the point and store the hit location
		if (hit.GetActor()) {

			//keep track of if the attached actor is the pylon, so can be unattached when it expires
			if (hit.GetActor() == currPylon) {
				attachedToPylon = true;
			}
			else {
				attachedToPylon = false;
			}

			grappleLerp = 0;
			grappleGoing = true;
			grapplePoint = hit.Location;

			//enable cable visibility and attach to wrist
			grappleCable->SetVisibility(true);
			grappleCable->SetAttachEndToComponent(handSlot);

			//get the difference to the hit point
			const FVector diff = hit.Location - GetActorLocation();

			//set the rotation of the hook end to the direction to the grapple point
			hook->SetActorRotation(UKismetMathLibrary::MakeRotFromX(-diff).GetEquivalentRotator() + FRotator(0,0,90));
		}
		else
		{
			//disable grapplehit
			grappleHit = false;
		}
	}
}

void APlayerCharacter::SecondaryReleased(const FInputActionValue& value)
{
	//disable the grapple being hit
	
	if (grappleHit || grappleGoing)
	{
		grappleHit = false;
		grappleGoing = false;
		//hide the cable component, reset cooldown of grapple ability
		grappleCable->SetVisibility(false);
		grappleTimer = GRAPPLECOOLDOWN;
	}
	
}

void APlayerCharacter::SecondaryInProgress(const FInputActionValue& value)
{

	//this function triggers every frame while the secondary (grapple) is still being held, swings them around the point through the use of plane projection

	//get difference of the player to the grapple point
	FVector diff =  GetActorLocation() - grapplePoint;

	//get the direction from the camera towards the grapple point as unit vector
	dirToPlayer =  grapplePoint - playerCam->GetComponentLocation();
	dirToPlayer.Normalize();

	//store length of the player difference to the point
	double diffL = diff.Length();

	//store velocity at point
	FVector velo = GetVelocity();

	//get dot product of the difference and the player's velocity
	double dot = diff.Dot(GetVelocity());

	//normalise diff vector
	diff.Normalize();
		
	//project the velcity to the plane of the normalised difference, and add the player's forward vector
	velo = FVector::VectorPlaneProject(velo, diff);
	velo += playerCam->GetForwardVector();

	// if the player's camera is facing the grappled point, move the player towards the grapple point
	if (dirToPlayer.Dot(playerCam->GetForwardVector()) >=0.99) {
		float strength = velo.Length();
		velo = (strength) * -diff;
	}
	
	//set the player's velocity to the calulated plane projected velocity if dot product is valid (greater than equal 0)
	if (dot >= 0  && grappleHit) {
		GetCharacterMovement()->Velocity = velo;
	}
}

void APlayerCharacter::Utility(const FInputActionValue& value)
{

	//if utility key is pressed, then if able to, set dash charging to be true
	if (utilityTimer <= 0) {
		dashCharging = true;
	}

}

void APlayerCharacter::UtilityRelease(const FInputActionValue& value)
{
	//set dash charging to false when dash key is released
	dashCharging = false;
}

void APlayerCharacter::Special(const FInputActionValue& value)
{
	//if special cooldown is over, when special key is pressed, spawn the pylon and pass camera look direction into it, reset cooldown

	if (specialTimer <= 0) {
		currPylon = GetWorld()->SpawnActor<APylon>(GetActorLocation() + (GetActorForwardVector() * 100), GetActorRotation());
		currPylon->setDir(playerCam->GetForwardVector());
		specialTimer = SPECIALCOOLDOWN;
	}
}

void APlayerCharacter::Sprint(const FInputActionValue& value)
{
	
	//toggle sprint when the spring key is pressed by changing the maximum walk speed
	if (!sprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = 700 * 1.45;
		sprinting = true;
	}else
	{
		sprinting = false;
		GetCharacterMovement()->MaxWalkSpeed = 700;
	}
}

void APlayerCharacter::TickDownTimers(float DeltaTime)
{
	//tick down cooldown timers for abilities
	if (primaryTimer > 0)
	{
		primaryTimer -= DeltaTime;
	}
	
	if (utilityTimer > 0) 
	{
		utilityTimer -= DeltaTime;
	}

	if (grappleTimer > 0) {
		grappleTimer -= DeltaTime;
	}

	if (specialTimer > 0) {
		specialTimer -= DeltaTime;
	}

	//tick down the timer for the dash duration
	if (dashActiveTimer > 0) {
		dashActiveTimer -= DeltaTime;
	}
}

void APlayerCharacter::TickCheckRange()
{
	//check to see if a grapple would be valid
	FHitResult hit;
	GetWorld()->LineTraceSingleByChannel(hit, playerCam->GetComponentLocation() + playerCam->GetForwardVector() * 100, playerCam->GetComponentLocation() + (playerCam->GetForwardVector() * GRAPPLERANGE), ECC_Visibility);

	if (hit.GetActor())
	{
		inRange = true;
	}
	else
	{
		inRange = false;
	}
}

void APlayerCharacter::TickHandleAttacking(float DeltaTime)
{
	//if the player is attacking, tick up timer until it reaches point in animation when the punch would hit
	if (attacking) {
		hurtTimer += DeltaTime;
		if (hurtTimer >= PDAMAGEPOINT) {
			if (!hasDamaged) {
				hasDamaged = true;

				//apply damage to all enemies overlapping the hurtbox
				for (int i = 0; i < overlappingEnemies.Num(); i++) {
					overlappingEnemies[i]->reduceHealth(DAMAGE * 3);
				}
			}
		}

		//if animation would be finished per the timer, reset it
		if (hurtTimer >= PRESETPOINT) {
			hurtTimer = 0;
			hasDamaged = false;
			attacking = false;
		}
	}
}

void APlayerCharacter::TickHandleDashCharge(float DeltaTime)
{

	//if the utility ability is being used, increase the power of the dash, and add an impulse to shoot them forward when released
	if (dashCharging) {
		if (dashMeter < DASHLIMIT) {
			//charge up dashMeter at rate of 1500 a second
			dashMeter += DASHRATE * DeltaTime;
		}
	}
	else {
		if (dashMeter > 0) {
			//set cooldown for utility ability
			utilityTimer = UTILITYCOOLDOWN;
			
			//save the current friction to reapply later
			savedFriction = GetCharacterMovement()->GroundFriction;

			//add force impulse along the direction of the camera forward vector scaled to how full the meter is
			GetCharacterMovement()->AddImpulse(dashMeter * playerCam->GetForwardVector(), true);

			//reset dash meter and set dashing bool to true
			dashMeter = 0;
			dashing = true;

			//set timer for a second for dash to be a second long
			dashActiveTimer = 1;
		}
	}
}

void APlayerCharacter::TickHandleDashCurrent()
{
	//if dashing
	if (dashing)
	{
		//set friction to 0
		GetCharacterMovement()->GroundFriction = 0;

		//stop dashing after dash timer has finished, reset friction to the saved value
		if (dashActiveTimer <= 0) {
			dashing = false;
			GetCharacterMovement()->GroundFriction = savedFriction;
		}
	}
}

void APlayerCharacter::TickHandleGrappleCurrent(float DeltaTime)
{
	//if the grappling hook should be travelling
	if (grappleGoing)
	{

		//increase interpolation meter to move the hook and cable to hit point
		grappleLerp += DeltaTime;
		if (grappleLerp / (1 / GRAPPLESPEED) > 1)
		{
			//unattach the grapple 
			grappleGoing = false;
			grappleHit = true;
			grappleLerp = 0;

			//disable player's gravity so I can do m
			GetCharacterMovement()->GravityScale = 0;

			//set grapple cable and hook position to the hit point
			grappleCable->SetWorldLocation(grapplePoint);
			hook->SetActorLocation(grapplePoint);

			//if player's velocity is 0, add an impulse towards the hit point to shoot player towards
			if (GetVelocity().Length() <= 0) {
				GetCharacterMovement()->AddImpulse(grapplePoint - GetActorLocation() * 1, true);
			}
		}
		else
		{
			// interpolate the position of the hook object and cable to the shot at point until it arrives
			FVector lerpPos = FMath::Lerp(GetActorLocation(), grapplePoint, grappleLerp / (1 / GRAPPLESPEED));

			hook->SetActorLocation(lerpPos);
			grappleCable->SetWorldLocation(lerpPos);
		}
	}
	else
	{
		//if the grapple has been hit
		if (grappleHit)
		{
			//apply new manual gravity value to the player's velocity
			GetCharacterMovement()->Velocity += (FVector(0, 0, -MANUALGRAVITY) * DeltaTime);
			//set cable length to slightly below the difference to keep it tense
			grappleCable->CableLength = FVector(grapplePoint - GetActorLocation()).Length() - 100;

			//if the grapple point is the pylon, disattach it when the pylon expires
			if (attachedToPylon) {
				if (currPylon->IsActorBeingDestroyed()) {
					grappleHit = false;
				}
			}

		}
		else
		{
			//if the grapple is not hit, disable hook's collision and set position and location to that of the slot
			hook->SetActorEnableCollision(false);
			hook->SetActorLocation(handSlot->GetComponentLocation());
			hook->SetActorRotation(handSlot->GetComponentRotation());
			hook->AddActorLocalRotation(FQuat(FVector(0, 0, 1), PI));
			GetCharacterMovement()->GravityScale = 1;
			if (grappleCable->IsVisible())
			{
				grappleCable->SetVisibility(false);
			}
		}
	}
}


//hurtbox entry/exit events
void APlayerCharacter::OnBoxBeginOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& hit)
{
	//check if actor is an enemy
	if (OtherActor->ActorHasTag(TEXT("Enemy"))) {
		AAlien* alien = Cast<AAlien>(OtherActor);

		//if the player is dashing
		if (dashing) {
			if (!alien->getKnocked()) {

				//get the length of the player's current velocity
				FVector unit = GetCharacterMovement()->Velocity;
				float length = unit.Length();
				unit.Normalize();

				//clamp checked for speed to 20m/s
				if (length > 2000) { length = 2000; };

				//reduce enemy health by DAMAGE + 30% per m/s moving
				alien->reduceHealth(DAMAGE + ((length / 100) * (DAMAGE*0.3)));

				//knock the enemy back by the damage
				alien->GetCharacterMovement()->AddImpulse((unit * length), true);
				alien->setKnocked(true);
			}
		}

		//add the alien to array of overlapping enemies
		overlappingEnemies.AddUnique(alien);
		
	}
}

void APlayerCharacter::OnBoxEndOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& hit)
{
	//remove enemy from list of overlapping enemies
	if (OtherActor->ActorHasTag(TEXT("Enemy"))) {
		AAlien* alien = Cast<AAlien>(OtherActor);

		if (overlappingEnemies.Contains(alien)) {
			overlappingEnemies.Remove(alien);
		}

	}
}