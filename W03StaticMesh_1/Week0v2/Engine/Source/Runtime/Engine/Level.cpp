#include "Level.h"

void ULevel::Tick(float DeltaTime)
{
    for (AActor* Actor: Actors)
    {
        Actor->Tick(DeltaTime);
    }
}

