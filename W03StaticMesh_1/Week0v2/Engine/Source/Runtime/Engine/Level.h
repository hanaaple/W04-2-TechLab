#pragma once
#include "GameFramework/Actor.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

enum class ELevelState
{
    Play,
    Pause,
};

class ULevel : public UObject
{
    DECLARE_CLASS(ULevel, UObject)
public:
    ULevel() = default;
    
    ELevelState LevelState;
    TArray<AActor*> Actors;
    void Tick(float DeltaTime);
};
