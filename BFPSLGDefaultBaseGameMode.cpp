// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "BFPSLGDefaultBaseGameMode.h"
#include "BFPSLGDefaultBaseHUD.h"
#include "BFPSLGDefaultBaseCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABFPSLGDefaultBaseGameMode::ABFPSLGDefaultBaseGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("Blueprint'/Game/BalancedFPSLevelGeneratorAssets/Blueprints/Characters/DebugCharacter_BP.DebugCharacter_BP'"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ABFPSLGDefaultBaseHUD::StaticClass();
}
