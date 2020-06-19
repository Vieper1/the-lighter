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

public:
#pragma region INIT
	ABlock();
#pragma endregion

	
#pragma region COLLISION
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "1. Block")
		bool bLit;
	
	UFUNCTION(BlueprintCallable, Category = "1. block")
		void SetOverlapEnabled(const bool bShouldOverlap);
#pragma endregion
};
