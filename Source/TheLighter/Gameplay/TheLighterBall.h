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
	UPROPERTY(EditAnywhere, Category = "1. Config")
		bool ShowDebugTrace = false;
#pragma endregion


	


	

	
	
#pragma region MODE
	UPROPERTY(EditAnywhere, Category = "2. Mode")
		bool bDisableMovement;

	UPROPERTY(EditAnywhere, Category = "2. Mode")
		bool bDisableAirControl;

	UPROPERTY(EditAnywhere, Category = "2. Mode")
		bool bDisableJump;

	UPROPERTY(EditAnywhere, Category = "2. Mode")
		bool bDisableExitImpulse;
#pragma endregion








	
#pragma region MOVEMENT
public:
	UPROPERTY(EditAnywhere, Category = "3. Movement")
		float LateralForce;
	
	UPROPERTY(EditAnywhere, Category = "3. Movement")
		float RollTorque;
	
	UPROPERTY(EditAnywhere, Category = "3. Movement")
		float MaxAngularVelocity = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "3. Movement")
		bool bIsGrounded = false;
	
	UPROPERTY(EditAnywhere, Category = "3. Movement")
		float JumpImpulse;

	UPROPERTY(EditAnywhere, Category = "3. Movement")
		float ExitImpulse;

	UFUNCTION(BlueprintCallable, Category = "3. Movement")
		void ApplyExitImpulse();


private:
	FRotator LastRotation;
	float ForceMultiplier = 1000000.f;
	float ImpulseMultiplier = 1000.f;

protected:
	void MoveRight(float Val);
	void Jump();
#pragma endregion








	


	
#pragma region TRACER
	float TraceAngle = 45.f;
	UPROPERTY(EditAnywhere, Category = "4. Tracer", meta = (ClampMin = "0", ClampMax = "8"))
		int NumberOfTraces = 2;

	UPROPERTY(EditAnywhere, Category = "4. Tracer")
		float TraceLength = 1.0f;

	UPROPERTY(EditAnywhere, Category = "4. Tracer", meta = (ClampMin = "0.0", ClampMax = "30.0"))
		float TraceAngleCorrection = 0.0f;

	UPROPERTY(EditAnywhere, Category = "4. Tracer", meta = (ClampMin = "0.0"))
		float TraceGroundingThreshold = 1.0f;

	UPROPERTY(EditAnywhere, Category = "4. Tracer", meta = (ClampMin = "0.0"))
		float TraceGroundingSeparation = 1.0f;

		
	TArray<class ABlock*> LitSet;
	void TraceCollision();
	inline bool SetAdd(TArray<ABlock*> &arrayRef, class ABlock * actorRef);
	inline bool SetRemove(TArray<ABlock*>& arrayRef, class ABlock * actorRef);
	bool TraceGrounding();
#pragma endregion






	


#pragma region COLLISION
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
#pragma endregion






	

#pragma region INPUT
	bool QueryMouseInput(class APlayerController* playerController);
	bool QueryControllerInput(class APlayerController* playerController);
#pragma endregion







	


#pragma region BEGINPLAY & TICK
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
#pragma endregion
};