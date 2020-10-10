// Created by Vishal Naidu (GitHub: Vieper1) naiduvishal13@gmail.com | Vishal.Naidu@utah.edu
// Extended from the Standard Actor Template


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

	// Don't update the collision preset
	// Until the PlayerBall exits the collider

	TArray<AActor*> overlappingActors;
	MeshComp->GetOverlappingActors(overlappingActors);
	if (CurrentCollisionResponse != TargetCollisionResponse && !overlappingActors.Contains(GetWorld()->GetFirstPlayerController()->GetPawn()))
		SetCollisionMode(TargetCollisionResponse);
}

void ABlock::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UWorld* world = GetWorld();
	if (!world) return;

	APlayerController* playerController = world->GetFirstPlayerController();
	if (!playerController) return;

	APawn* playerPawn = playerController->GetPawn();
	if (playerPawn && OtherActor == playerPawn)
	{

		// Apply an Impulse to the PlayerBall after it exits
		// This allows us to do the HotWheels-Booster effect on the ball
		// When it passes through a series of LighterBlocks placed close to each other

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