// Fill out your copyright notice in the Description page of Project Settings.


#include "Alien.h"
#include "Components/CapsuleComponent.h"
#define ANIMSCALE 1.5

// Sets default values
AAlien::AAlien()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//tag as enemy
	Tags.Add(TEXT("Enemy"));

	//script delegates to bind events on entering/exiting hurtbox
	FScriptDelegate beginOverlapDelegate;
	beginOverlapDelegate.BindUFunction(this, "OnBoxBeginOverlap");

	FScriptDelegate endOverlapDelegate;
	endOverlapDelegate.BindUFunction(this, "OnBoxEndOverlap");

	//create collision box for damaging the player
	hurtBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Hurtbox"));
	hurtBox->SetBoxExtent(FVector(50, 50, 50));
	hurtBox->AddRelativeLocation(FVector(50, 0, 0));
	hurtBox->SetupAttachment(RootComponent);
	hurtBox->SetGenerateOverlapEvents(true);

	//bind delegates to entering/exiting hurtbox
	hurtBox->OnComponentBeginOverlap.AddUnique(beginOverlapDelegate);
	hurtBox->OnComponentEndOverlap.AddUnique(endOverlapDelegate);
	hurtBox->SetActive(false);

	//default health for enemy
	health = 80;

	//set the speed of the enemy
	GetCharacterMovement()->MaxWalkSpeed = 300.f;	

	//load the enemy's skeletal mesh and the animation blueprint being used
	const ConstructorHelpers::FObjectFinder<USkeletalMesh> mesh(TEXT("/Script/Engine.SkeletalMesh'/Game/UsedAnims/alien.alien'"));
	FindComponentByClass<USkeletalMeshComponent>()->SetSkeletalMesh(mesh.Object);

	const ConstructorHelpers::FObjectFinder<UAnimBlueprint> abp(TEXT("/Script/Engine.AnimBlueprint'/Game/Blueprints/ABP_AlienAnim.ABP_AlienAnim'"));
	FindComponentByClass<USkeletalMeshComponent>()->SetAnimInstanceClass(abp.Object->GeneratedClass);
}

// Called when the game starts or when spawned
void AAlien::BeginPlay()
{
	Super::BeginPlay();

	//store the player character, to be used to movwe towards and attack
	APlayerController* con = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	player = con->GetPawn();
}

// Called every frame
void AAlien::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//if not dead
	if (!dead) {

		//get the distance to the player
		FVector dist = player->GetActorLocation() - GetActorLocation();

		//normalise the distance as a direction unit vector
		FVector dir = dist;
		dir.Z = 0;
		dir.Normalize();

		//add the unit vector as a movement input, using the movement component to move towards to player
		GetCharacterMovement()->AddInputVector(dir);

		//rotate to to face tha player
		SetActorRotation(UKismetMathLibrary::MakeRotFromX(dir).GetEquivalentRotator());

		//process being knocked back so dash attack only hits once
		if (knockTimer <= 0) {
			knocked = false;
		}
		else {
			knockTimer -= DeltaTime;
		}


		//if the enemy is within a meter of the player, start attacking
		if (dist.Length() <= 100) {
			
			//attacking variable both used for checks and animating correctly
			attacking = true;
			hurtTimer += DeltaTime;

			//if timer reaches the point where the punch happens in animation, check if player is in hurtbox and then call damage response function
			if (hurtTimer >= DAMAGEPOINT/ANIMSCALE) {
				if (!hasHit) {
					hasHit = true;
					if (playerOverlap) {
						Cast<APlayerCharacter>(player)->damageResponse(ADAMAGE*2);
					}
				}
			}

			//if timer reaches point where animation loops, reset timer
			if (hurtTimer >= RESETPOINT / ANIMSCALE) {
				hurtTimer = 0;
				hasHit = false;
			}
		}
		else {
			//if not attacking any more, reset timer
			hurtTimer = 0;
			hasHit = false;
			attacking = false;
		}

		//check if dead and set state accordingly, initialise the rotting timer
		if (health <= 0) {
			dead = true;
			rotTimer = 0;
		}
	}
	else {
		//tick up rotting timer if dead and destroy body after 5 seconds
		rotTimer += DeltaTime;

		if (GetVelocity().Length() <= 0) {
			//disable collision on this enemy's capsule component
			FindComponentByClass<UCapsuleComponent>()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (rotTimer >= 5) {
			Destroy();
		}
	}


}

// Called to bind functionality to input
void AAlien::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AAlien::OnBoxBeginOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& hit)
{
	//set the player to being inside the hurt box
	if (OtherActor->ActorHasTag(TEXT("Player"))) {
		playerOverlap = true;
	}
}

void AAlien::OnBoxEndOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& hit)
{
	//set the player to being outside the hurt box
	if (OtherActor->ActorHasTag(TEXT("Player"))) {

		playerOverlap = false;
	}
}

bool AAlien::getKnocked()
{
	return knocked;
}

void AAlien::setKnocked(bool nKnock)
{
	//set knockback state and if true, reset the timer
	knocked = nKnock;
	if (nKnock) {
		knockTimer = 1.f;
	}
}

void AAlien::reduceHealth(int reduction)
{
	//reduce health accordingly
	health -= reduction;
}
