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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ball, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* Ball;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ball, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ball, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* Camera;
#pragma endregion

	
public:
	ATheLighterBall();



	
#pragma region MODE
	UPROPERTY(EditAnywhere, Category = "Mode")
		bool bDisableAirControl;

	UPROPERTY(EditAnywhere, Category = "Mode")
		bool bDisableJump;

	UPROPERTY(EditAnywhere, Category = "Mode")
		bool bDisableMovement;
#pragma endregion

#pragma region CONFIG
	UPROPERTY(EditAnywhere, Category = "Mode")
		UMaterialInterface * BallMaterial;
#pragma endregion




#pragma region BEGINPLAY & TICK
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
#pragma endregion




	
#pragma region MOVEMENT
public:
	bool bCanJump;
	UPROPERTY(EditAnywhere, Category=Ball)
		float JumpImpulse;

	UPROPERTY(EditAnywhere, Category=Ball)
		float RollTorque;

protected:
	void MoveRight(float Val);
	void MoveForward(float Val);
	void Jump();
#pragma endregion
	




#pragma region COLLISION
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
#pragma endregion
};
