// Created by Vishal Naidu (GitHub: Vieper1) naiduvishal13@gmail.com | Vishal.Naidu@utah.edu
// Extending Unreal's Pawn class to gain PlayerControl features

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TheLighterBall.generated.h"


// Events to be fired upon DoubleJump & ExitImpulse
// This is because I want to do particle emissions on the blueprint side to keep things clean
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDoubleJumpDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FExitImpulseDelegate);


// Used for climb prevention due to high friction
UENUM()
enum WallingDirection
{
	Left,
	Right,
	Both,
	None
};




////////////////////////////////////////////////////////////////////// CORE
UCLASS(config=Game)
class ATheLighterBall : public APawn
{
	GENERATED_BODY()

#pragma region CORE COMPONENTS

	// Using the Ball preset from StarterContent

	// Push the SpringArm length so far, and the Camera FOV so small
	// That you get as close to 2D as possible

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core", meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* Ball;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core", meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core", meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* Camera;

	// This is the magic
	// Lumino's Flashlight that can toggle collisions on Lighter Blocks
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core", meta = (AllowPrivateAccess = "true"))
		class USpotLightComponent* SpotLight;

public:
	UFUNCTION(BlueprintCallable, Category = "Core")
		FORCEINLINE class UStaticMeshComponent* GetMesh() const { return Ball; }
#pragma endregion







	
#pragma region INIT
public:
	ATheLighterBall();
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
#pragma endregion
////////////////////////////////////////////////////////////////////// CORE










// Set DEADZONES & Debug Toggles

////////////////////////////////////////////////////////////////////// INPUT TOGGLES
#pragma region CONFIG
public:
	// Toggles all the LineTraces to show up during gameplay
	UPROPERTY(EditAnywhere, Category = "////////// 1. Config")
		bool bShowDebugTrace = false;

	// Apply mouse input only when it moves
	UPROPERTY(EditAnywhere, Category = "////////// 1. Config", meta = (ClampMin = "0.0"))
		float MouseInputThreshold = 0.3f;

	// Controller DeadZone
	UPROPERTY(EditAnywhere, Category = "////////// 1. Config", meta = (ClampMin = "0.0"))
		float GamepadInputThreshold = 0.3f;

	// PlayerBall's gravity
	UPROPERTY(EditAnywhere, Category = "////////// 1. Config", meta = (ClampMin = "0.1"))
		float GravityMultiplier = 1.0f;
#pragma endregion
////////////////////////////////////////////////////////////////////// INPUT CONFIG



	


	

	


////////////////////////////////////////////////////////////////////// INPUT TOGGLES
#pragma region MODE
	// Pause control over the flashlight
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "////////// 2. Mode")
		bool bDisableTracerControl;
	
	// Pause inputs to the PlayerBall controller
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "////////// 2. Mode")
		bool bDisableMovement;

	// Disable using Mid-Air-Forces
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "////////// 2. Mode")
		bool bDisableAirControl;

	// Disable jumping
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "////////// 2. Mode")
		bool bDisableJump;

	// Disable the boost the PlayerBall gets when it exits a LighterBlock
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "////////// 2. Mode")
		bool bDisableExitImpulse;

	// Unpossess PlayerBall controller
	UFUNCTION(BlueprintCallable, Category = "////////// 2. Mode")
		void DetachPlayerBall();
#pragma endregion
////////////////////////////////////////////////////////////////////// INPUT TOGGLES











// This is the config you want to change when you want to tune the BALL'S HANDLING

////////////////////////////////////////////////////////////////////// MOVEMENT CONFIG
#pragma region MOVEMENT
public:
	// Side-to-side force
	UPROPERTY(EditAnywhere, Category = "////////// 3. Movement", meta = (ClampMin = "0.0"))
		float LateralForce;
	
	// Additional roll torque to apply while pushing to the side
	UPROPERTY(EditAnywhere, Category = "////////// 3. Movement", meta = (ClampMin = "0.0"))
		float RollTorque;
	
	UPROPERTY(EditAnywhere, Category = "////////// 3. Movement", meta = (ClampMin = "0.0"))
		float MaxAngularVelocity = 0.0f;

	// Is PlayerBall on ground?
	UPROPERTY(BlueprintReadOnly, Category = "////////// 3. Movement")
		bool bIsGrounded = false;

	// Is PlayerBall close to a wall?
	UPROPERTY(BlueprintReadOnly, Category = "////////// 3. Movement")
		bool bIsWalled = false;

	// Set velocity for a consistent jump
	UPROPERTY(EditAnywhere, Category = "////////// 3. Movement", meta = (ClampMin = "0.0"))
		float BaseJumpVelocity;

	UPROPERTY(EditAnywhere, Category = "////////// 3. Movement", meta = (ClampMin = "0.0"))
		float DoubleJumpVelocity;

	// The actual ExitImpulse to apply when exiting a LighterBlock
	UPROPERTY(EditAnywhere, Category = "////////// 3. Movement", meta = (ClampMin = "0.0"))
		float ExitImpulse;

	// Force@ExitNormal - to - Force@PlayerBallVelocityDirection => ratio
	UPROPERTY(EditAnywhere, Category = "////////// 3. Movement", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float ExitImpulseRatio;

	UPROPERTY(EditAnywhere, Category = "////////// 3. Movement", meta = (ClampMin = "0.0"))
		float MaxExitVelocity;

	// Time delay between last grounding and jump input for double jump
	UPROPERTY(EditAnywhere, Category = "////////// 3. Movement", meta = (ClampMin = "0.0"))
		float DoubleJumpThreshold = 1.0f;


	// Exit Impulse is the impulse that's applied when the PlayerBall gets out of a LighterBlock mesh
	UFUNCTION(BlueprintCallable, Category = "////////// 3. Movement")
		void ApplyExitImpulse();


private:
	FRotator LastTargetRotation;
	float ForceMultiplier = 1000000.f;
	float ImpulseMultiplier = 1000.f;
#pragma endregion
////////////////////////////////////////////////////////////////////// MOVEMENT CONFIG







	

////////////////////////////////////////////////////////////////////// TRACER
// The Tracer assists in checking the following:
// 1. LineTrace N lines to check if PlayerBall is lighting up a LighterBlock
// 2. Is PlayerBall super close to a wall?

#pragma region TRACER
	float TraceAngle = 45.f;

	// Number of LineTraceByChannels
	UPROPERTY(EditAnywhere, Category = "////////// 4. Tracer", meta = (ClampMin = "0", ClampMax = "8"))
		int NumberOfTraces = 2;

	UPROPERTY(EditAnywhere, Category = "////////// 4. Tracer")
		float TraceLength = 1.0f;

	UPROPERTY(EditAnywhere, Category = "////////// 4. Tracer")
		float TracerSpeed = 1.0f;

	// Angle correction for SpotLight cone and Tracer angle
	UPROPERTY(EditAnywhere, Category = "////////// 4. Tracer", meta = (ClampMin = "0.0", ClampMax = "30.0"))
		float TraceAngleCorrection = 0.0f;

	// Separation from center to check for walls
	UPROPERTY(EditAnywhere, Category = "////////// 4. Tracer", meta = (ClampMin = "0.0"))
		float TraceWallingThreshold = 1.0f;

	// Distance below PlayerBall to check for floor
	UPROPERTY(EditAnywhere, Category = "////////// 4. Tracer", meta = (ClampMin = "0.0"))
		float TraceGroundingThreshold = 1.0f;

	// Distance between the two traces (In case we're on sloped floors, we don't want to prevent the player from jumping)
	UPROPERTY(EditAnywhere, Category = "////////// 4. Tracer", meta = (ClampMin = "0.0"))
		float TraceGroundingSeparation = 1.0f;

	// Distance towards the camera from the PlayerBall (For correction only)
	UPROPERTY(EditAnywhere, Category = "////////// 4. Tracer", meta = (ClampMin = "0.0"))
		float TraceForwardCorrection = 1.0f;



	// Functions to SMOOTHLY LERP the Flashlight in the intended direction
	void SetTracerRotation(const FVector Direction);				// Set target flashlight rotation
	void LerpTracerToTargetRotation(const float DeltaSeconds);		// Rotate flashlight smoothly





	// This is the set of DATA STRUCTURES
	// That help drive our TRACER ALGORITHM - To help query & store ACTIVELY LIT LighterBlocks

	TArray<class ABlock*> LitSet;
	void TraceCollision();											// Fire traces to check LighterBlocks
	inline bool SetAdd(TArray<ABlock*> &arrayRef, class ABlock * actorRef, const bool bCollisionToggle);		// Data structure to handle active LighterBLocks
	inline bool SetRemove(TArray<ABlock*>& arrayRef, class ABlock * actorRef, const bool bCollisionToggle);		// Data structure to handle active LighterBLocks
	
	bool TraceGrounding();											// Trace for IsGrounded
	WallingDirection TraceWalling();								// Direction in which the PlayerBall is close to a wall





protected:
	// Event to fire when player DoubleJumps
	float GroundedTime = 0.0f;
	UPROPERTY(BlueprintAssignable, Category = "Test")
		FDoubleJumpDelegate OnDoubleJump;

	// Event to fire when PlayerBall exits a LighterBlock
	UPROPERTY(BlueprintAssignable, Category = "Test")
		FExitImpulseDelegate OnExitImpulse;
	
	UPROPERTY(BlueprintReadOnly)
		FRotator CurrentTracerRotation;
	UPROPERTY(BlueprintReadOnly)
		FRotator TargetTracerRotation;
#pragma endregion
////////////////////////////////////////////////////////////////////// TRACER





	


#pragma region COLLISION
public:
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
#pragma endregion






	
////////////////////////////////////////////////////////////////////// INPUT & CONTROL
#pragma region INPUT
public:
	UFUNCTION(BlueprintCallable, Category = "Input")
		void DisablePlayerInput();
	UFUNCTION(BlueprintCallable, Category = "Input")
		void EnablePlayerInput();
protected:
	float InputGamepadRX = 0.f;
	float InputGamepadRY = 0.f;
	
	// Inputs to map
	void MoveRight(float Val);
	void PointRight(float Val);
	void PointUp(float Val);
	void Jump();

	
	// Query mouse input per tick and differentiate between controller and mouse input
	bool QueryMouseInput(class APlayerController* playerController);
	bool QueryGamepadInput(class APlayerController* playerController);

private:
	FVector LastPointerLocation;
#pragma endregion
////////////////////////////////////////////////////////////////////// INPUT & CONTROL









	

////////////////////////////////////////////////////////////////////// EVENTS
#pragma region BEGINPLAY & TICK
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
#pragma endregion
};
////////////////////////////////////////////////////////////////////// EVENTS