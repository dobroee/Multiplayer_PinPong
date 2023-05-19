// Fill out your copyright notice in the Description page of Project Settings.


#include "PingPongPlatform.h"

// Sets default values
APingPongPlatform::APingPongPlatform()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	BodyCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Platfrom Body Collider"));
	SetRootComponent(BodyCollision);
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Platform Body Mesh"));
	BodyMesh->SetupAttachment(RootComponent);
	BodyMesh->SetIsReplicated(true);
	SetReplicates(true);
	SetReplicateMovement(true);

}

// Called when the game starts or when spawned
void APingPongPlatform::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APingPongPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APingPongPlatform::Server_MoveRight_Implementation(float AxisValue)
{
	if(AxisValue != 0)
    {
	    UE_LOG(LogTemp, Warning, TEXT("APingPongPlatform::Server_MoveRight_Implementation"));
    }
    FVector currLocation = GetActorLocation();
    FVector nextLocation = GetActorLocation() + GetActorRightVector() * MoveSpeed * AxisValue;
    if(!SetActorLocation(nextLocation, true))
    {
		SetActorLocation(currLocation);
    }

}

bool APingPongPlatform::Server_MoveRight_Validate(float AxisValue)
{
	return true;
}

