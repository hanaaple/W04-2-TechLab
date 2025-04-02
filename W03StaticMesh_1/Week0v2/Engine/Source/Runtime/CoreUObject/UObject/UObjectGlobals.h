#pragma once
#include "NameTypes.h"
#include "Container/Map.h"
#include "Engine/EngineTypes.h"

enum class EObjectFlags;
class UObject;
class UClass;

enum class EDuplicateMode
{
    Normal,
    World,
    PIE
};

inline TCHAR to_string(const EDuplicateMode e)
{
    switch (e)
    {
    case EDuplicateMode::Normal: return TEXT(*"Normal");
    case EDuplicateMode::World: return TEXT(*"World");
    case EDuplicateMode::PIE: return TEXT(*"PIE");
    default: return TEXT(*"unknown");
    }
}