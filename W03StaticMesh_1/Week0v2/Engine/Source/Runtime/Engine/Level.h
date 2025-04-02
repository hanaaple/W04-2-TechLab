#pragma once
#include "GameFramework/Actor.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class ULevel : public UObject
{
    DECLARE_CLASS(ULevel, UObject)
    
public:
    TArray<AActor*> Actors;
};
