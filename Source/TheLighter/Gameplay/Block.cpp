// Fill out your copyright notice in the Description page of Project Settings.


#include "Block.h"

#pragma region INIT
ABlock::ABlock()
{
	UStaticMeshComponent* meshComp = GetStaticMeshComponent();
	meshComp->SetCollisionProfileName(FName("LighterBlock"));
}
#pragma endregion



#pragma region COLLISION
void ABlock::SetCollisionMode(const ECollisionResponse collisionResponse)
{
	UStaticMeshComponent* meshComp = GetStaticMeshComponent();

	meshComp->SetCollisionResponseToChannel(ECC_Pawn, collisionResponse);
	meshComp->SetCollisionResponseToChannel(ECC_PhysicsBody, collisionResponse);

	CurrentCollisionResponse = collisionResponse;
}
#pragma endregion