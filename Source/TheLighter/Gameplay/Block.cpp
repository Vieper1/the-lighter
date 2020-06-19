// Fill out your copyright notice in the Description page of Project Settings.


#include "Block.h"

ABlock::ABlock()
{
	UStaticMeshComponent* meshComp = GetStaticMeshComponent();
	meshComp->SetCollisionProfileName(FName("LighterBlock"));
}