// Copyright Epic Games, Inc. All Rights Reserved.

#include "Rouge_like_gameGameMode.h"
#include "Rouge_like_gameCharacter.h"
#include "UObject/ConstructorHelpers.h"

ARouge_like_gameGameMode::ARouge_like_gameGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
