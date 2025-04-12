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

// UObject* ULevel::Duplicate()
// {
//     ULevel* DuplicatedLevel = Cast<ULevel>(FObjectFactory::DuplicateObject(this, this->GetClass()));
//     TArray<AActor*> Origin = this->Actors;
//     TArray<AActor*> DuplicatedActors;
//     // (&DuplicatedLevel)
//     DuplicatedLevel->Actors.Empty();
//     for (int i = 0; i < Actors.Num(); i++)
//     {
//         // DuplicatedActors.Add(Cast<AActor>(Actors[i]->Duplicate()));
//         AActor* abc = Cast<AActor>(Actors[i]->Duplicate());
//         DuplicatedActors.Add(abc);
//     }
//
//     DuplicatedLevel->Actors = DuplicatedActors;
//     this->Actors = Origin;
//     return DuplicatedLevel;
// }

