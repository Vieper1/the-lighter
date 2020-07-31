// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TheLighterBall.generated.h"

UCLASS(config=Game)
class ATheLighterBall : public APawn
{
	GENERATED_BODY()

#pragma region CORE COMPONENTS
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core", meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* Ball;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core", meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core", meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* Camera;
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




	
#pragma region CONFIG
public:
	UPROPERTY(EditAnywhere, Category = "////////// 1. Config")
		bool bShowDebugTrace = false;

	UPROPERTY(EditAnywhere, Category = "////////// 1. Config", meta = (ClampMin = "0.0"))
		float MouseInputThreshold = 0.3f;

	UPROPERTY(EditAnywhere, Category = "////////// 1. Config", meta = (ClampMin = "0.0"))
		float GamepadInputThreshold = 0.3f;

	UPROPERTY(EditAnywhere, Category = "////////// 1. Config", meta = (ClampMin = "0.1"))
		float GravityMultiplier = 1.0f;
#pragma endregion


	


	

	
	
#pragma region MODE
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "////////// 2. Mode")
		bool bDisableTracerControl;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "////////// 2. Mode")
		bool bDisableMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "////////// 2. Mode")
		bool bDisableAirControl;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "////////// 2. Mode")
		bool bDisableJump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "////////// 2. Mode")
		bool bDisableExitImpulse;

	UFUNCTION(BlueprintCallable, Category = "////////// 2. Mode")
		void DetachPlayerBall();
#pragma endregion








	
#pragma region MOVEMENT
public:
	UPROPERTY(EditAnywhere, Category = "////////// 3. Movement", meta = (ClampMin = "0.0"))
		float TerminalVelocity;
	
	UPROPERTY(EditAnywhere, Category = "////////// 3. Movement", meta = (ClampMin = "0.0"))
		float LateralForce;
	
	UPROPERTY(EditAnywhere, Category = "////////// 3. Movement", meta = (ClampMin = "0.0"))
		float RollTorque;
	
	UPROPERTY(EditAnywhere, Category = "////////// 3. Movement", meta = (ClampMin = "0.0"))
		float MaxAngularVelocity = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "////////// 3. Movement")
		bool bIsGrounded = false;

	UPROPERTY(BlueprintReadOnly, Category = "////////// 3. Movement")
		bool bIsWalled = false;
	
	UPROPERTY(EditAnywhere, Category = "////////// 3. Movement", meta = (ClampMin = "0.0"))
		float JumpImpulse;

	UPROPERTY(EditAnywhere, Category = "////////// 3. Movement", meta = (ClampMin = "0.0"))
		float ExitImpulse;

	UFUNCTION(BlueprintCallable, Category = "////////// 3. Movement")
		void ApplyExitImpulse();


private:
	FRotator LastTargetRotation;
	float ForceMultiplier = 1000000.f;
	float ImpulseMultiplier = 1000.f;
#pragma endregion








	


	
#pragma region TRACER
	float TraceAngle = 45.f;
	UPROPERTY(EditAnywhere, Category = "////////// 4. Tracer", meta = (ClampMin = "0", ClampMax = "8"))
		int NumberOfTraces = 2;

	UPROPERTY(EditAnywhere, Category = "////////// 4. Tracer")
		float TraceLength = 1.0f;

	UPROPERTY(EditAnywhere, Category = "////////// 4. Tracer")
		float TracerSpeed = 1.0f;

	UPROPERTY(EditAnywhere, Category = "////////// 4. Tracer", meta = (ClampMin = "0.0", ClampMax = "30.0"))
		float TraceAngleCorrection = 0.0f;

	UPROPERTY(EditAnywhere, Category = "////////// 4. Tracer", meta = (ClampMin = "0.0"))
		float TraceGroundingThreshold = 1.0f;
	
	UPROPERTY(EditAnywhere, Category = "////////// 4. Tracer", meta = (ClampMin = "0.0"))
		float TraceWallingThreshold = 1.0f;

	UPROPERTY(EditAnywhere, Category = "////////// 4. Tracer", meta = (ClampMin = "0.0"))
		float TraceGroundingSeparation = 1.0f;

	UPROPERTY(EditAnywhere, Category = "////////// 4. Tracer", meta = (ClampMin = "0.0"))
		float TraceForwardCorrection = 1.0f;

		
	TArray<class ABlock*> LitSet;
	void TraceCollision();
	void SetTracerRotation(const FVector Direction);
	void LerpTracerToTargetRotation(const float DeltaSeconds);
	inline bool SetAdd(TArray<ABlock*> &arrayRef, class ABlock * actorRef, const bool bCollisionToggle);
	inline bool SetRemove(TArray<ABlock*>& arrayRef, class ABlock * actorRef, const bool bCollisionToggle);
	bool TraceGrounding();
	bool TraceWalling();

protected:
	UPROPERTY(BlueprintReadOnly)
		FRotator CurrentTracerRotation;
	UPROPERTY(BlueprintReadOnly)
		FRotator TargetTracerRotation;
#pragma endregion






	


#pragma region COLLISION
public:
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
#pragma endregion






	

#pragma region INPUT
protected:
	float InputGamepadRX = 0.f;
	float InputGamepadRY = 0.f;
	
	void MoveRight(float Val);
	void PointRight(float Val);
	void PointUp(float Val);
	void Jump();

	bool QueryMouseInput(class APlayerController* playerController);
	bool QueryGamepadInput(class APlayerController* playerController);

private:
	FVector LastPointerLocation;
#pragma endregion







	


#pragma region BEGINPLAY & TICK
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
#pragma endregion
};