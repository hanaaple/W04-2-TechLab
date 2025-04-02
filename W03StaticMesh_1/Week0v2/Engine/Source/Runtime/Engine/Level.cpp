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

UObject* ULevel::Duplicate()
{
    ULevel* DuplicatedLevel = Cast<ULevel>(FObjectFactory::DuplicateObject(this));
    DuplicatedLevel->Actors.Empty();
    for (AActor* Actor: Actors)
    {
        DuplicatedLevel->Actors.Add(Cast<AActor>(Actor->Duplicate()));
    }
    return DuplicatedLevel;
}

