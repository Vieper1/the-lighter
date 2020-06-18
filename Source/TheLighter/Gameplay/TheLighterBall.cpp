// Copyright Epic Games, Inc. All Rights Reserved.

#include "TheLighterBall.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Components/SpotLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"



#pragma region INIT
ATheLighterBall::ATheLighterBall()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> BallMesh(TEXT("/Game/Rolling/Meshes/BallMesh.BallMesh"));

	// Create mesh component for the ball
	Ball = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ball0"));
	Ball->SetStaticMesh(BallMesh.Object);
	Ball->BodyInstance.SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	Ball->SetSimulatePhysics(true);
	Ball->SetAngularDamping(0.1f);
	Ball->SetLinearDamping(0.1f);
	Ball->BodyInstance.MassScale = 3.5f;
	Ball->BodyInstance.MaxAngularVelocity = 800.0f;
	Ball->SetNotifyRigidBodyCollision(true);
	RootComponent = Ball;

	// Constraint
	Ball->BodyInstance.bLockXRotation = true;
	Ball->BodyInstance.bLockZRotation = true;
	Ball->BodyInstance.bLockXTranslation = true;
	Ball->SetConstraintMode(EDOFMode::YZPlane);

	// Create a camera boom attached to the root (ball)
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bDoCollisionTest = false;
	SpringArm->SetUsingAbsoluteRotation(true); // Rotation of the ball should not affect rotation of boom
	SpringArm->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	SpringArm->TargetArmLength = 1200.f;
	SpringArm->bEnableCameraLag = false;
	SpringArm->CameraLagSpeed = 3.f;

	// Create a camera and attach to boom
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // We don't want the controller rotating the camera

	// Spotlight
	SpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLight0"));
	SpotLight->SetupAttachment(RootComponent);
	SpotLight->SetRelativeRotation(FRotator(0, 90, 0));
	SpotLight->SetUsingAbsoluteRotation(true);

	
	// Set up forces
	RollTorque = 50.f;
	RollTorqueMultiplier = 1000000.f;
	JumpImpulse = 350.0f;
	bCanJump = true;
}

void ATheLighterBall::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	PlayerInputComponent->BindAxis("MoveRight", this, &ATheLighterBall::MoveRight);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ATheLighterBall::Jump);
}
#pragma endregion



#pragma region BEGINPLAY & TICK
void ATheLighterBall::BeginPlay()
{
	Super::BeginPlay();

	// Initial Config
	if (MaxAngularVelocity > 0.f)
		Ball->SetPhysicsMaxAngularVelocityInRadians(MaxAngularVelocity);

	TraceAngle = SpotLight->OuterConeAngle - TraceAngleCorrection;

	APlayerController * player0 = GetWorld()->GetFirstPlayerController();
	if (player0) player0->Possess(this);
}
void ATheLighterBall::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);


	// Input
	APlayerController * playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	FVector mouseLocation;
	FVector mouseDirection;
	const bool bMouseQuerySuccess = playerController->DeprojectMousePositionToWorld(mouseLocation, mouseDirection);
	const bool bControllerQuerySuccess = false;
	if (bMouseQuerySuccess)
	{
		const float HIT_TEST_DISTANCE = 10000.f;

		FHitResult hit;
		GetWorld()->LineTraceSingleByChannel(hit, mouseLocation, mouseLocation + mouseDirection * HIT_TEST_DISTANCE, ECollisionChannel::ECC_Visibility);
		const FVector mouseWorldLocation = hit.bBlockingHit ? hit.ImpactPoint : hit.TraceEnd;

		const FVector actorLocation = GetActorLocation();
		const FVector spotLightDirection = UKismetMathLibrary::GetDirectionUnitVector(FVector(0, actorLocation.Y, actorLocation.Z), FVector(0, mouseWorldLocation.Y, mouseWorldLocation.Z));
		const FRotator spotLightRotation = UKismetMathLibrary::MakeRotFromX(spotLightDirection);
		
		SpotLight->SetWorldRotation(spotLightRotation);
		LastRotation = spotLightRotation;
	}

	if (!bMouseQuerySuccess && !bControllerQuerySuccess)
	{
		SpotLight->SetWorldRotation(LastRotation);
	}

	// Trace
	TraceCollision();
}
#pragma endregion











#pragma region INPUT
void ATheLighterBall::MoveRight(float Val)
{
	const FVector Torque = FVector(-1.f * Val * RollTorque * RollTorqueMultiplier, 0.f, 0.f);
	Ball->AddTorqueInRadians(Torque);
}

void ATheLighterBall::Jump()
{
	if(bCanJump)
	{
		const FVector Impulse = FVector(0.f, 0.f, JumpImpulse);
		Ball->AddImpulse(Impulse);
		bCanJump = false;
	}
}
#pragma endregion



#pragma region SCANNER
void ATheLighterBall::TraceCollision()
{
	TArray<FHitResult> hitArray;

	const FRotator spotLightRotation = SpotLight->GetComponentRotation();
	

	for (int i = 0; i < NumberOfTraces; i++)
	{
		if (ShowDebugTraces)
		{
			const FRotator lineRotation = UKismetMathLibrary::ComposeRotators(spotLightRotation, FRotator(0, 0, -TraceAngle + (TraceAngle * 2 * i / (NumberOfTraces - 1))));
			DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + lineRotation.Vector() * TraceLength, FColor::Red);
		}
	}
}
#pragma endregion





#pragma region COLLISION
void ATheLighterBall::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
	bCanJump = true;
}
#pragma endregion
