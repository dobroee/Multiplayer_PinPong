// Fill out your copyright notice in the Description page of Project Settings.


#include "PingPongBall.h"

#include "DrawDebugHelpers.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystem.h"

// Sets default values
APingPongBall::APingPongBall()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	BodyCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Ball BodyCollider"));
	SetRootComponent(BodyCollision);
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ball BodyMesh"));
	BodyMesh->SetupAttachment(RootComponent);
	BodyMesh->SetIsReplicated(true);
	SetReplicates(true);
	SetReplicateMovement(true);
	
	/*static ConstructorHelpers::FObjectFinder<UStaticMesh>
		LoadedBallMeshObj(TEXT("/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere"));
	BodyMesh->SetStaticMesh(LoadedBallMeshObj.Object);*/
	

}

// Called when the game starts or when spawned
void APingPongBall::BeginPlay()
{
	Super::BeginPlay();

	/*BodyMesh->SetStaticMesh(LoadBodyMesh());
	BodyMesh->SetMaterial(0, LoadMaterial());*/
	LoadBodyMesh();
	LoadEffect();
	
}

void APingPongBall::Server_StartMove_Implementation()
{
	IsMoving = true;
}

bool APingPongBall::Server_StartMove_Validate()
{
	return true;
}

void APingPongBall::Server_Move_Implementation(float DeltaTime)
{
	FVector forward = GetActorForwardVector();
    FVector currLoc = GetActorLocation();
    FVector newLoc = currLoc + forward * MoveSpeed * DeltaTime;
    FHitResult hitResult;
    if(!SetActorLocation(newLoc, true, &hitResult))
    {
	    UE_LOG(LogTemp, Warning, TEXT("Ball %s Collided with %s"), *GetName(), *hitResult.GetActor()->GetName());
	    FVector moveVector = forward - currLoc;
	    moveVector.Normalize();
	    FVector resetPosition = currLoc + moveVector * DeltaTime * 5 * MoveSpeed;
	    DrawDebugDirectionalArrow(GetWorld(), newLoc + moveVector * 300, newLoc, 30, FColor::Yellow, true, 3.f, 0, 3);
	    FVector impactCorrection = hitResult.ImpactPoint + hitResult.ImpactNormal * 300;
	    DrawDebugDirectionalArrow(GetWorld(), hitResult.ImpactPoint,
			hitResult.ImpactPoint + hitResult.ImpactNormal * 300, 30, FColor::Blue, true, 3, 0, 3);
	    float AimAtAngle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(moveVector, hitResult.ImpactNormal)));
	    moveVector = moveVector.RotateAngleAxis(AimAtAngle*2, FVector(0,0,1));
	    FVector newTargetMove = newLoc + moveVector * 300;
	    newTargetMove.Z = currLoc.Z;
	    DrawDebugDirectionalArrow(GetWorld(), newLoc, newTargetMove, 30, FColor::Yellow, true, 3.f, 0, 3);
	    //SetActorLocation(currLoc);
	    SetActorLocation(resetPosition);
	    FRotator currRotation = GetActorRotation();
	    FRotator newRotation = UKismetMathLibrary::FindLookAtRotation(currLoc, newTargetMove);
	    newRotation.Pitch = currRotation.Pitch;
	    //newRotation.Yaw = newRotation.Yaw + FMath::RandRange(-10, 10);
	    SetActorRotation(newRotation);
	    Multicast_HitEffect();
    }

}

bool APingPongBall::Server_Move_Validate(float DeltaTime)
{
	return true;
}

void APingPongBall::Server_StopMove_Implementation()
{
	IsMoving = false;
}

bool APingPongBall::Server_StopMove_Validate()
{
	return true;
}

void APingPongBall::Multicast_HitEffect_Implementation()
{
	UWorld * world = GetWorld();
    if(world && HitEffect)
    {
	    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEffect, GetActorLocation());
    }

}

/*UStaticMesh* APingPongBall::LoadBodyMesh()
{
	if (BodyMeshRef.IsPending())
    {
	    const FSoftObjectPath& AssetRef = BodyMeshRef.ToStringReference();
	    FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
	    BodyMeshRef = Cast<
	    UStaticMesh>(StreamableManager.LoadSynchronous(AssetRef));
    }
    return BodyMeshRef.Get();

	/*UStaticMesh* Mesh = LoadObject<UStaticMesh>(NULL,
		TEXT("/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere"), NULL, LOAD_None, NULL);
    return Mesh;#1#
}*/

UMaterialInterface* APingPongBall::LoadMaterial()
{
	if (MaterialRef.IsPending())
	{
		const FSoftObjectPath& AssetRef = MaterialRef.ToStringReference();
		FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
		MaterialRef = Cast<
		UMaterialInterface>(StreamableManager.LoadSynchronous(AssetRef));
	}
	return MaterialRef.Get();
	
	/*UMaterialInterface* Material = LoadObject<UMaterialInstance>(NULL,
		TEXT("/DatasmithContent/Materials/FBXImporter/DeltaGenMaster.DeltaGenMaster"), NULL, LOAD_None, NULL);
	return Material;*/
}

void APingPongBall::LoadBodyMesh()
{
	FStreamableDelegate LoadMeshDelegate;
	LoadMeshDelegate.BindUObject(this, &APingPongBall::OnBodyMeshLoaded);
	UAssetManager& assetManager = UAssetManager::Get();
	FStreamableManager& streamableManager = assetManager.GetStreamableManager();
	AssetHandle = streamableManager.RequestAsyncLoad(BodyMeshRef.ToStringReference(), LoadMeshDelegate);
}

void APingPongBall::OnBodyMeshLoaded()
{
	UStaticMesh * loadedMesh = Cast<UStaticMesh>(AssetHandle.Get()->GetLoadedAsset());
    if(loadedMesh)
    {
		BodyMesh->SetStaticMesh(loadedMesh);
    }

}

void APingPongBall::LoadEffect()
{
	FStreamableDelegate LoadEffectDelegate;
	LoadEffectDelegate.BindUObject(this, &APingPongBall::OnEffectLoaded);
	UAssetManager& assetManager = UAssetManager::Get();
	FStreamableManager& streamableManager = assetManager.GetStreamableManager();
	AssetHandle = streamableManager.RequestAsyncLoad(EffectRef.ToStringReference(), LoadEffectDelegate);
}

void APingPongBall::OnEffectLoaded()
{
	UParticleSystem * loadedEffect = Cast<UParticleSystem>(AssetHandle.Get()->GetLoadedAsset());
	if(loadedEffect)
	{
		//BodyMesh->SetStaticMesh(loadedMesh);
		HitEffect = loadedEffect;
	}
}

// Called every frame
void APingPongBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetNetMode() != ENetMode::NM_Client)
	{
		Server_Move(DeltaTime);
	}

}

void APingPongBall::StartMove()
{
	Server_StartMove();
}

void APingPongBall::StopMove()
{
	Server_StopMove();
}

void APingPongBall::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(APingPongBall, IsMoving);
}

