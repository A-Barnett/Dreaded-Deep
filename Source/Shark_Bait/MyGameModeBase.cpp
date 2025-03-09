#include "MyGameModeBase.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Blueprint.h"
#include "Engine/World.h"

/***********************************************

MyGameModeBase.cpp - Alex Barnett
Basic Gamemode Setup, Spawns Player

***********************************************/

AMyGameModeBase::AMyGameModeBase()
{
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/MyCharacter_B"));
    if (PlayerPawnBPClass.Class != nullptr)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
    }
}
