// Copyright Epic Games, Inc. All Rights Reserved.

#include "TheLighterGameMode.h"
#include "TheLighterBall.h"

ATheLighterGameMode::ATheLighterGameMode()
{
	// set default pawn class to our ball
	DefaultPawnClass = ATheLighterBall::StaticClass();
}
