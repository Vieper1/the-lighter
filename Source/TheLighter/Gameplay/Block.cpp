// Fill out your copyright notice in the Description page of Project Settings.


#include "Block.h"

#pragma region CORE
ABlock::ABlock()
{
	MeshComp = GetStaticMeshComponent();
	MeshComp->SetCollisionProfileName(FName("LighterBlock"));
	MeshComp->SetGenerateOverlapEvents(true);
}
#pragma endregion



#pragma region COLLISION
void ABlock::SetCollisionMode(const ECollisionResponse CollisionResponse)
{
	UStaticMeshComponent* meshComp = GetStaticMeshComponent();

	meshComp->SetCollisionResponseToChannel(ECC_Pawn, CollisionResponse);
	meshComp->SetCollisionResponseToChannel(ECC_PhysicsBody, CollisionResponse);

	CurrentCollisionResponse = CollisionResponse;
}
#pragma endregion




#pragma region EVENTS
void ABlock::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TArray<AActor*> overlappingActors;
	MeshComp->GetOverlappingActors(overlappingActors);
	if (CurrentCollisionResponse != TargetCollisionResponse && overlappingActors.Num() == 0)
		SetCollisionMode(TargetCollisionResponse);
}
#pragma endregion