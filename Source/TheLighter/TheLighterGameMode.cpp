// Copyright Epic Games, Inc. All Rights Reserved.

#include "TheLighterGameMode.h"
#include "Gameplay/TheLighterBall.h"

ATheLighterGameMode::ATheLighterGameMode()
{
	// set default pawn class to our ball
	DefaultPawnClass = ATheLighterBall::StaticClass();
}
