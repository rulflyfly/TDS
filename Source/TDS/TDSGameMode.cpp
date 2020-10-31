// Copyright Epic Games, Inc. All Rights Reserved.

#include "TDSGameMode.h"
#include "TDSPlayerController.h"
#include "Character/TDSCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATDSGameMode::ATDSGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ATDSPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Bluprint/Character/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}


