// Fill out your copyright notice in the Description page of Project Settings.

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		class UStaticMeshComponent* MeshComp;
#pragma endregion

	

	
#pragma region COLLISION
private:
	ECollisionResponse CurrentCollisionResponse;
	void SetCollisionMode(const ECollisionResponse CollisionResponse);
	
public:
	ECollisionResponse TargetCollisionResponse;

	UFUNCTION()
		void OnComponentEndOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
#pragma endregion



	
#pragma region EVENTS
public:
	
	virtual void Tick(float DeltaSeconds) override;
#pragma endregion
};
