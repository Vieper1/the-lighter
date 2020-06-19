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
void ABlock::SetOverlapEnabled(const bool bShouldOverlap)
{
	UStaticMeshComponent* meshComp = GetStaticMeshComponent();

	FCollisionResponseContainer container;
	container.SetResponse(ECC_Pawn, bShouldOverlap ? ECR_Overlap : ECR_Block);
	container.SetResponse(ECC_PhysicsBody, bShouldOverlap ? ECR_Overlap : ECR_Block);
	meshComp->SetCollisionResponseToChannels(container);

	bLit = bShouldOverlap;
}
#pragma endregion