#pragma once
#include "Container/Array.h"
#include "Container/Map.h"
#include "Container/Set.h"

class UClass;
class UObject;


class FUObjectArray
{
public:
    void AddObject(UObject* Object);
    void MarkRemoveObject(UObject* Object);

    UObject* GetObjectByUUID(int32 UUID);

    void ProcessPendingDestroyObjects();

    TMap<uint32, UObject*>& GetObjectItemArrayUnsafe()
    {
        return ObjObjects;
    }

    const TMap<uint32, UObject*>& GetObjectItemArrayUnsafe() const
    {
        return ObjObjects;
    }

private:
    //TODO : UUID 별로 Array에 추가되는 게 확실하다면 TArray로 변경
    TMap<uint32, UObject*> ObjObjects;
    TArray<UObject*> PendingDestroyObjects;
};

extern FUObjectArray GUObjectArray;
