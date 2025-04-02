#include "Level.h"

void ULevel::Tick(float DeltaTime)
{
    if (LevelState != ELevelState::Play)
        return;
    for (AActor* Actor: Actors)
    {
        Actor->Tick(DeltaTime);
    }
}

