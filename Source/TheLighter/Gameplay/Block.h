// Created by Vishal Naidu (GitHub: Vieper1) naiduvishal13@gmail.com | Vishal.Naidu@utah.edu
// Extended from the Standard Actor template

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Block.generated.h"

/**
 * 
 */
UCLASS(config=Game)
class ABlock : public AStaticMeshActor
{
	GENERATED_BODY()

#pragma region CORE
public:
	ABlock();

	// The Cubes we're going to be using as the LighterBlocks
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class UStaticMeshComponent* MeshComp;
#pragma endregion

	

	
#pragma region COLLISION
private:

	// This function is used to provide a LateUpdate to the LighterBlock's collision preset
	ECollisionResponse CurrentCollisionResponse;
	void SetCollisionMode(const ECollisionResponse CollisionResponse);
	
public:
	// This is the collision preset we want
	ECollisionResponse TargetCollisionResponse;

	// Overriding the EndOverlap so we could update the collision preset after the ball exits
	UFUNCTION()
		void OnComponentEndOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
#pragma endregion



	
#pragma region EVENTS
public:
	virtual void Tick(float DeltaSeconds) override;
#pragma endregion
};
