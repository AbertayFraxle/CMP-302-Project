// Fill out your copyright notice in the Description page of Project Settings.


#include "Pylon.h"
#include "Alien.h"
// Sets default values
APylon::APylon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	//load the	model, set it to the correct orientation and set as root component
	model = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Model"));
	const ConstructorHelpers::FObjectFinder<UStaticMesh> mesh(TEXT("/Script/Engine.StaticMesh'/Game/Models/pylon.pylon'"));
	model->SetStaticMesh(mesh.Object);
	model->SetRelativeRotation(FQuat(FVector(1,0,0),-3.14/2));
	model->SetWorldScale3D(FVector(25,25,25));
	SetRootComponent(model);	

	//set up the delegates to set the sphere entry and exit functions
	FScriptDelegate beginOverlapDelegate;
	beginOverlapDelegate.BindUFunction(this, "OnSphereBeginOverlap");

	FScriptDelegate endOverlapDelegate;
	endOverlapDelegate.BindUFunction(this, "OnSphereEndOverlap");

	//load the niagara FX system from the project resources
	const ConstructorHelpers::FObjectFinder<UNiagaraSystem> nSys(TEXT("/Script/Niagara.NiagaraSystem'/Game/vfx/zapSystem.zapSystem'"));
	beamTemp = nSys.Object;

	//set hurtSphere setup for hitting enemies
	hurtSphere = CreateDefaultSubobject <USphereComponent>(TEXT("Hurt Sphere"));
	hurtSphere->SetSphereRadius(20);
	hurtSphere->SetupAttachment(RootComponent);
	hurtSphere->SetGenerateOverlapEvents(true);

	//bind delegates to entry/exit functions
	hurtSphere->OnComponentBeginOverlap.Add(beginOverlapDelegate);
	hurtSphere->OnComponentEndOverlap.Add(endOverlapDelegate);

	//initialise deployment timer and also the counter to kill object
	counter = 0;
	deployTimer = 1;
}

// Called when the game starts or when spawned
void APylon::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APylon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//rotate the pylon
	angle = (PI / 5) * DeltaTime;
	AddActorWorldRotation(FQuat(FVector(0, 0, 1), angle));

	//if deploying
	if (deployTimer > 0) {

		//tick down the deploy timer
		deployTimer -= DeltaTime;
		
		//use a raycats to check if the actor can move in the direction specified without clipping into a wall
		FHitResult hit;
		GetWorld()->LineTraceSingleByChannel(hit, GetActorLocation(), GetActorLocation()+ (dir * DEPLOYSPEED * DeltaTime), ECC_Visibility);

		//if theres a hit, change direction according to the hit normal
		if (hit.GetActor()) {
			dir = hit.ImpactNormal;
			dir.Normalize();
		}

		//move the actor according to the direction
		SetActorLocation(GetActorLocation() + (dir * DEPLOYSPEED * DeltaTime));
	}
	else {

		//tick up timer before attack
		zapTimer += DeltaTime;

		//if it's been a second
		if (zapTimer >= 1) {

			//reset the timer
			zapTimer = 0;

			//for all enemies overlapping the hurtSphere, hit them for the specified damage amount
			for (int i = 0; i < overlappingEnemies.Num(); i++) {
				overlappingEnemies[i]->reduceHealth(ZAPDAMAGE);

				//create a beam component to hit the enemy
				UNiagaraComponent* newBeam;
				newBeam = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), beamTemp, GetActorLocation());
				newBeam->SetNiagaraVariableVec3(FString("NewEnd"), overlappingEnemies[i]->GetActorLocation());
				newBeam->SetAutoDestroy(true);

			}
			//upount the counter, and if its above the threshold, destroy this pylon
			counter++;
			if (counter >= SHOTS) {
				Destroy();
			}
		}
	}

}

void APylon::setDir(FVector nDir)
{

	//set direction to new direction
	dir = nDir;
}

//on entering the hurtSphere, add enemies to the array to deal damage when the pylon fires its attack
void APylon::OnSphereBeginOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& hit)
{
	if (OtherActor->ActorHasTag(TEXT("Enemy"))) {
		AAlien* alien = Cast<AAlien>(OtherActor);
		overlappingEnemies.AddUnique(alien);
	}
}

//on exiting the hurtSphere, add remove enemis from array
void APylon::OnSphereEndOverlap(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& hit)
{
	if (OtherActor->ActorHasTag(TEXT("Enemy"))) {
		AAlien* alien = Cast<AAlien>(OtherActor);

		if (overlappingEnemies.Contains(alien)) {
			overlappingEnemies.Remove(alien);
		}

	}
}