// Fill out your copyright notice in the Description page of Project Settings.


#include "Block.h"
#include "TheLighterBall.h"

#pragma region CORE
ABlock::ABlock()
{
	MeshComp = GetStaticMeshComponent();
	MeshComp->SetCollisionProfileName(FName("LighterBlock"));
	MeshComp->SetGenerateOverlapEvents(true);
	MeshComp->SetMobility(EComponentMobility::Stationary);
	MeshComp->OnComponentEndOverlap.AddDynamic(this, &ABlock::OnComponentEndOverlap);
}
#pragma endregion







#pragma region EVENTS
void ABlock::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TArray<AActor*> overlappingActors;
	MeshComp->GetOverlappingActors(overlappingActors);
	if (CurrentCollisionResponse != TargetCollisionResponse && !overlappingActors.Contains(GetWorld()->GetFirstPlayerController()->GetPawn()))
		SetCollisionMode(TargetCollisionResponse);


	GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Cyan, FString::Printf(TEXT("%d"), TargetCollisionResponse));
}

void ABlock::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* playerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (playerPawn && OtherActor == playerPawn)
	{
		ATheLighterBall* playerBall = Cast<ATheLighterBall>(playerPawn);
		playerBall->ApplyExitImpulse();
	}
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