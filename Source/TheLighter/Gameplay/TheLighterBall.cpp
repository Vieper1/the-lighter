// Created by Vishal Naidu (GitHub: Vieper1) naiduvishal13@gmail.com | Vishal.Naidu@utah.edu
// Extending Unreal's Pawn class to gain PlayerControl features

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
#include "Block.h"
#include "DrawDebugHelpers.h"



////////////////////////////////////////////////////////////////////// CORE
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
}

void ATheLighterBall::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	PlayerInputComponent->BindAxis("MoveRight", this, &ATheLighterBall::MoveRight);
	PlayerInputComponent->BindAxis("PointRight", this, &ATheLighterBall::PointRight);
	PlayerInputComponent->BindAxis("PointUp", this, &ATheLighterBall::PointUp);
	
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ATheLighterBall::Jump);
}
#pragma endregion INIT
////////////////////////////////////////////////////////////////////// CORE













////////////////////////////////////////////////////////////////////// EVENTS
#pragma region BEGINPLAY & TICK
void ATheLighterBall::BeginPlay()
{
	Super::BeginPlay();

	// Initial Config
	if (MaxAngularVelocity > 0.f)
		Ball->SetPhysicsMaxAngularVelocityInRadians(MaxAngularVelocity);

	// Set Tracer cone angle on play start
	TraceAngle = SpotLight->OuterConeAngle - TraceAngleCorrection;
	TargetTracerRotation = FRotator(0, 90, 0);

	DrawDebugSphere(GetWorld(), LastPointerLocation, 100.f, 64, FColor::Red);
}





void ATheLighterBall::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Query for Tracer Control
	APlayerController * playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!bDisableTracerControl)
	{
		QueryMouseInput(playerController);
		QueryGamepadInput(playerController);
	}

	// Use SMOOTH interpolation to rotate the flashlight
	LerpTracerToTargetRotation(DeltaSeconds);


	// Invoke the TRACER-ALGORITHM
	TraceCollision();


	// Jump Toggle
	bIsGrounded = TraceGrounding();

	// Gravity Correction
	Ball->AddForce(FVector::DownVector * GravityMultiplier);

	// Double Jump Logic
	if (bIsGrounded)
		GroundedTime += DeltaSeconds;
	else
		GroundedTime = 0.0f;
}
#pragma endregion BEGINPLAY & TICK
////////////////////////////////////////////////////////////////////// EVENTS


















// Tracer

/* 
*			+-----------------------+
* 			| 	  THE TECHNIQUE		|
* 			+-----------------------+
* 
* 1. Since there are multiple line traces (which reduces trace misses), we'd need to add an 
*  	 EXTRA DATA STRUCTURE to handle how many LighterBlocks we've hit in one group trace
* 	 It's called the "HITSET"
* 
* 2. HitSet in turn controls what objects we have in a list that contains the actual LIT ITEMS
* 	 It's called the "LITSET"
* 
* 3. We need the LitSet because we need to toggle the collisions on the LigterBlocks ONLY ONCE
* 
* 
* NOTE: 	HitSet	=> It is PER-LINE sensitive
* 			LitSet	=> It is PER-FRAME sensitive
* 
* 
* 
* 
* 
* 
* 
* 
* 
* Algorithm
* ---------
* 1. Trace out N lines along your SpotLight's Cone
* 
* 2. If you hit a LighterBlock on any of the traces
* 		a. Call the ToggleCollision function on it to enable collision
* 		b. Add it to the list of Lit Items
* 
* 3. If all of your traces fail to find the LighterBlock
* 		a. Call the ToggleCollision again to disable collision
* 		b. Remove it from the list
* 
* 
* NOTE: The ToggleCollision function only sets the target collision,
* 		But the real collision change occurs when NOTHING's overlapping the LighterBlock
*/





////////////////////////////////////////////////////////////////////// TRACER
#pragma region TRACER
void ATheLighterBall::TraceCollision()
{
	const FRotator spotLightRotation = SpotLight->GetComponentRotation();
	TArray<ABlock*> hitSet;

	
	// EVENLY ANGLED LINE TRACES
	// To populate the HITSET

	for (int i = 0; i < NumberOfTraces; i++)
	{
		const FRotator lineRotation = UKismetMathLibrary::ComposeRotators(spotLightRotation, FRotator(0, 0, -TraceAngle + (TraceAngle * 2 * i / (NumberOfTraces - 1))));
		const FVector traceStart = GetActorLocation();
		const FVector traceEnd = GetActorLocation() + lineRotation.Vector() * TraceLength;

		FHitResult outHit;
		GetWorld()->LineTraceSingleByChannel(outHit, traceStart, traceEnd, ECollisionChannel::ECC_GameTraceChannel1);

		if (bShowDebugTrace)
			DrawDebugLine(
				GetWorld(), 
				traceStart + FVector::BackwardVector * TraceForwardCorrection, 
				outHit.bBlockingHit ? outHit.ImpactPoint + FVector::BackwardVector * TraceForwardCorrection : traceEnd + FVector::BackwardVector * TraceForwardCorrection,
				FColor::Red);

		if (outHit.bBlockingHit)
			SetAdd(hitSet, Cast<ABlock>(outHit.GetActor()), false);
	}
	// Populate HITSET
	



	// Tracer Algorithm
	if (LitSet.Num() == 0)
	{
		for (ABlock* hitActor : hitSet)
			SetAdd(LitSet, hitActor, true);
	}
	else
	{
		for (int i = 0; i < LitSet.Num(); ++i)
		{
			if (!hitSet.Contains(LitSet[i]))
			{
				SetRemove(LitSet, LitSet[i], true);
				continue;
			}
			for (ABlock* hitActor : hitSet)
			{
				SetAdd(LitSet, hitActor, true);
			}
		}
	}
	// Tracer Algorithm
}

bool ATheLighterBall::SetAdd(TArray<ABlock*>& arrayRef, ABlock* actorRef, const bool bCollisionToggle)
{
	if (!arrayRef.Contains(actorRef))
	{
		if (bCollisionToggle && actorRef->TargetCollisionResponse != ECR_Block)
			actorRef->TargetCollisionResponse = ECR_Block;
		arrayRef.Add(actorRef);
		return true;
	}
	return false;
}

bool ATheLighterBall::SetRemove(TArray<ABlock*>& arrayRef, ABlock* actorRef, const bool bCollisionToggle)
{
	if (arrayRef.Contains(actorRef))
	{
		if (bCollisionToggle && actorRef->TargetCollisionResponse != ECR_Overlap)
			actorRef->TargetCollisionResponse = ECR_Overlap;
		arrayRef.Remove(actorRef);
		return true;
	}
	return false;
}







// Wrote my own IsOnGround toggle

// This trace tells us if the PlayerBall is allowed to jump
// PREVENTS the JUMP-SKIP when the PlayerBall is on a SLOPE while MOVING FAST

bool ATheLighterBall::TraceGrounding()
{
	UWorld* world = GetWorld();
	const FVector startLocation = GetActorLocation();
	const FVector rightTraceLocation = GetActorLocation() + (FVector::UpVector * -TraceGroundingThreshold) + (FVector::RightVector * TraceGroundingSeparation);
	const FVector leftTraceLocation = rightTraceLocation + (FVector::RightVector * -2.f * TraceGroundingSeparation);

	if (bShowDebugTrace)
	{
		DrawDebugLine(world, startLocation + FVector::BackwardVector * TraceForwardCorrection, rightTraceLocation + FVector::BackwardVector * TraceForwardCorrection, FColor::Red);
		DrawDebugLine(world, startLocation + FVector::BackwardVector * TraceForwardCorrection, leftTraceLocation + FVector::BackwardVector * TraceForwardCorrection, FColor::Red);
	}
	
	FHitResult leftHit;
	world->LineTraceSingleByChannel(leftHit, startLocation, leftTraceLocation, ECC_Visibility);

	FHitResult rightHit;
	world->LineTraceSingleByChannel(rightHit, startLocation, rightTraceLocation, ECC_Visibility);

	if (leftHit.bBlockingHit || rightHit.bBlockingHit)
		return true;
	
	return false;
}









// This is a separate trace just to tell if we're close to the walls
// Useful to PREVENT WALL CLIMB since we're applying LATERAL FORCES

WallingDirection ATheLighterBall::TraceWalling()
{
	UWorld* world = GetWorld();
	const FVector startLocation = GetActorLocation();
	const FVector rightTraceLocation = GetActorLocation() + (FVector::RightVector * TraceWallingThreshold);
	const FVector leftTraceLocation = rightTraceLocation + (FVector::RightVector * -2.f * TraceWallingThreshold);

	if (bShowDebugTrace)
	{
		DrawDebugLine(world, startLocation + FVector::BackwardVector * TraceForwardCorrection, rightTraceLocation + FVector::BackwardVector * TraceForwardCorrection, FColor::Red);
		DrawDebugLine(world, startLocation + FVector::BackwardVector * TraceForwardCorrection, leftTraceLocation + FVector::BackwardVector * TraceForwardCorrection, FColor::Red);
	}

	FHitResult leftHit;
	world->LineTraceSingleByChannel(leftHit, startLocation, leftTraceLocation, ECC_Visibility);

	FHitResult rightHit;
	world->LineTraceSingleByChannel(rightHit, startLocation, rightTraceLocation, ECC_Visibility);

	if (leftHit.bBlockingHit && rightHit.bBlockingHit)
		return WallingDirection::Both;
	else if (leftHit.bBlockingHit)
		return WallingDirection::Left;
	else if (rightHit.bBlockingHit)
		return WallingDirection::Right;
		
	return WallingDirection::None;
}


// Set tracer rotation smoothly
void ATheLighterBall::SetTracerRotation(const FVector Direction)
{
	const FRotator spotLightRotation = UKismetMathLibrary::MakeRotFromX(Direction);

	//SpotLight->SetWorldRotation(spotLightRotation);
	TargetTracerRotation = spotLightRotation;
	LastTargetRotation = spotLightRotation;
}

void ATheLighterBall::LerpTracerToTargetRotation(const float DeltaSeconds)
{
	const FRotator spotLightRotation = SpotLight->GetComponentRotation();
	const FRotator newRotation = UKismetMathLibrary::RInterpTo(spotLightRotation, TargetTracerRotation, DeltaSeconds, TracerSpeed);
	SpotLight->SetWorldRotation(FRotator(newRotation.Pitch, newRotation.Yaw, 0));
}
#pragma endregion TRACER
////////////////////////////////////////////////////////////////////// TRACER




















////////////////////////////////////////////////////////////////////// INPUTS & MOVEMENT
#pragma region INPUTS AND MOVEMENT


////////////////////////////////////////////////// Input Toggles
void ATheLighterBall::DisablePlayerInput()
{
	bDisableMovement = true;
	bDisableJump = true;
	bDisableTracerControl = true;
}

void ATheLighterBall::EnablePlayerInput()
{
	bDisableMovement = false;
	bDisableJump = false;
	bDisableTracerControl = false;
}
////////////////////////////////////////////////// Input Toggles






////////////////////////////////////////////////// Ball Movement Control
void ATheLighterBall::MoveRight(float Val)
{
	if (bDisableMovement) return;

	const FVector Force = FVector(0, Val * LateralForce * ForceMultiplier, 0);
	if (bIsGrounded)
		Ball->AddForce(Force);
	else
		Ball->AddForce(bDisableAirControl ? FVector::ZeroVector : Force);
}

void ATheLighterBall::Jump()
{
	if (bDisableMovement || bDisableJump) return;

	
	if (bIsGrounded)
	{
		const FVector ballVelocity = GetVelocity();
		if (GroundedTime < DoubleJumpThreshold)
		{
			Ball->SetPhysicsLinearVelocity(FVector(ballVelocity.X, ballVelocity.Y, DoubleJumpVelocity));
			OnDoubleJump.Broadcast();
		}
		else
			Ball->SetPhysicsLinearVelocity(FVector(ballVelocity.X, ballVelocity.Y, BaseJumpVelocity));
	}
}
////////////////////////////////////////////////// Ball Movement Control







////////////////////////////////////////////////// Tracer Control

/*
* Manually querying MOUSE and GAMEPAD
* for input gives us more control over how to switch between the two
*
* This FIXES the problem of the FLASHLIGHT FLICKING AROUND
* when players have both connected
*/




// MANUAL QUERY (Tick)
// To check where the mouse is currently
// Use the relative location to provide a LOOK ANGLE
// For the flashlight

bool ATheLighterBall::QueryMouseInput(APlayerController* playerController)
{
	float deltaX;
	float deltaY;
	playerController->GetInputMouseDelta(deltaX, deltaY);
	if (fabs(deltaX) < MouseInputThreshold && fabs(deltaY) < MouseInputThreshold)
		return false;
	
	FVector mouseLocation;
	FVector mouseDirection;
	const bool bMouseQuerySuccess = playerController->DeprojectMousePositionToWorld(mouseLocation, mouseDirection);
	if (bMouseQuerySuccess)
	{
		const float HIT_TEST_DISTANCE = SpringArm->TargetArmLength;

		FHitResult hit;
		GetWorld()->LineTraceSingleByChannel(hit, mouseLocation, mouseLocation + mouseDirection * HIT_TEST_DISTANCE, ECollisionChannel::ECC_Visibility);
		const FVector mouseWorldLocation = hit.TraceEnd;
		LastPointerLocation = mouseWorldLocation;

		const FVector actorLocation = GetActorLocation();
		const FVector spotLightDirection = UKismetMathLibrary::GetDirectionUnitVector(FVector(0, actorLocation.Y, actorLocation.Z), FVector(0, mouseWorldLocation.Y, mouseWorldLocation.Z));
		SetTracerRotation(spotLightDirection);
		
		return true;
	}
	return false;
}





/*
* Use a 2-LAYER-BUFFER for gamepad inputs
*
* To help SEPARATE the inputs coming from mouse & gamepad
*/


// MANUAL QUERY (Tick)
// Query any GamePads for input
// Feed the right stick direction to the inputs

void ATheLighterBall::PointRight(float Val) { InputGamepadRX = Val; }
void ATheLighterBall::PointUp(float Val) { InputGamepadRY = Val; }


bool ATheLighterBall::QueryGamepadInput(APlayerController* playerController)
{
	if (fabs(InputGamepadRX) < GamepadInputThreshold && fabs(InputGamepadRY) < GamepadInputThreshold)
		return false;
	
	const FVector actorLocation = GetActorLocation();
	const FVector spotLightDirection = FVector(0, InputGamepadRX, InputGamepadRY);
	SetTracerRotation(spotLightDirection);
	
	return true;
}
void ATheLighterBall::DetachPlayerBall()
{
	bDisableTracerControl = true;
	bDisableMovement = true;
	bDisableAirControl = true;
	bDisableJump = true;

	GetWorld()->GetFirstPlayerController()->bShowMouseCursor = true;
}
////////////////////////////////////////////////// Tracer Control








////////////////////////////////////////////////// Exit Impulse
void ATheLighterBall::ApplyExitImpulse()
{
	const FVector ballVelocity = GetVelocity();
	const FVector spotLightDirection = SpotLight->GetForwardVector() * -1;
	const float angle = FMath::Acos(FVector::DotProduct(ballVelocity.GetSafeNormal(), spotLightDirection));

	const float multiplier = ExitImpulse * ImpulseMultiplier;
	const FVector velocityBoost = ballVelocity.GetSafeNormal() * multiplier;
	const FVector spotlightBoost = spotLightDirection * multiplier;

	if (!bDisableExitImpulse)
		Ball->AddImpulse(velocityBoost * ExitImpulseRatio + spotlightBoost * (1 - ExitImpulseRatio));
		

	
	if (ballVelocity.Size() > MaxExitVelocity)
		Ball->SetPhysicsLinearVelocity(ballVelocity.GetSafeNormal() * MaxExitVelocity);

	OnExitImpulse.Broadcast();
}
////////////////////////////////////////////////// Exit Impulse



#pragma endregion INPUTS AND MOVEMENT
////////////////////////////////////////////////////////////////////// INPUTS & MOVEMENT
















////////////////////////////////////////////////////////////////////// COLLISION
#pragma region COLLISION
void ATheLighterBall::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
}
#pragma endregion COLLISION
////////////////////////////////////////////////////////////////////// COLLISION