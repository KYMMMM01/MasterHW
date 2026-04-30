// Copyright Epic Games, Inc. All Rights Reserved.

#include "MasterHWGameMode.h"
#include "MasterHWCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMasterHWGameMode::AMasterHWGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
