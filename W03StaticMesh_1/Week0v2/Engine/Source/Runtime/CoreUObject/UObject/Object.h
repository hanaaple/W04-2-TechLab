#pragma once
#include "NameTypes.h"
#include "Container/Map.h"

class FEditorEngine;
extern FEditorEngine GEngineLoop;

class UClass;
class UWorld;
class FVector4;

struct FDuplicateContext
{
    // UObject 포인터를 키로 하여, 이미 복제된 UObject를 저장합니다.
    TMap<const class UObject*, class UObject*> DuplicateMap;
};

class UObject
{
private:
    UObject(const UObject&) = delete;
    UObject& operator=(const UObject&) = delete;
    UObject(UObject&&) = delete;
    UObject& operator=(UObject&&) = delete;
    
public:
    using Super = UObject;
    using ThisClass = UObject;

    static UClass* StaticClass();

private:
    friend class FObjectFactory;
    friend class FSceneMgr;
    friend class UClass;

    uint32 UUID;
    uint32 InternalIndex; // Index of GUObjectArray

    FName NamePrivate;
    UClass* ClassPrivate = nullptr;
    UObject* OuterPrivate = nullptr;


public:
    UObject();
    virtual ~UObject() = default;

    UWorld* GetWorld();

    FEditorEngine& GetEngine();

    FName GetFName() const;
    FString GetName() const;

    UObject* GetOuter() const { return OuterPrivate; }

    uint32 GetUUID() const;
    uint32 GetInternalIndex() const;
    UClass* GetClass() const;

    /** this가 SomeBase인지, SomeBase의 자식 클래스인지 확인합니다. */
    bool IsA(const UClass* SomeBase) const;
    UObject* Duplicate();

    virtual UObject* Duplicate(FDuplicateContext& Context);

    template <typename T>
        requires std::derived_from<T, UObject>
    bool IsA() const
    {
        return IsA(T::StaticClass());
    }

public:
    // void* operator new(size_t size);

    void operator delete(void* ptr, size_t size);

    FVector4 EncodeUUID() const;
    
private:
};
