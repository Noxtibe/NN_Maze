// Copyright Epic Games, Inc. All Rights Reserved.

#include "NN_MazeGameMode.h"
#include "NN_MazePlayerController.h"
#include "NN_MazeCharacter.h"
#include "UObject/ConstructorHelpers.h"

ANN_MazeGameMode::ANN_MazeGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ANN_MazePlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set default controller to our Blueprinted controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownPlayerController"));
	if(PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}