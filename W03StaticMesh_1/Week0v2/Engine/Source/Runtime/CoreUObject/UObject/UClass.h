#pragma once
#include <concepts>

#include "Object.h"
#include "Engine/Source/Runtime/Core/Container/Set.h"

/**
 * UObject의 RTTI를 가지고 있는 클래스
 */
class UClass : public UObject
{
public:
    using ObjectCreator = UObject*(*)();
    
    UClass(const char* InClassName, uint32 InClassSize, uint32 InAlignment, UClass* InSuperClass, ObjectCreator InCreator);
    virtual ~UClass() override = default;
    FName GetDefaultObjectName() const;

    // 복사 & 이동 생성자 제거
    UClass(const UClass&) = delete;
    UClass& operator=(const UClass&) = delete;
    UClass(UClass&&) = delete;
    UClass& operator=(UClass&&) = delete;

    uint32 GetClassSize() const { return ClassSize; }
    uint32 GetClassAlignment() const { return ClassAlignment; }

    /** SomeBase의 자식 클래스인지 확인합니다. */
    bool IsChildOf(const UClass* SomeBase) const;

    template <typename T>
        requires std::derived_from<T, UObject>
    bool IsChildOf() const
    {
        return IsChildOf(T::StaticClass());
    }

    /**
     * 부모의 UClass를 가져옵니다.
     *
     * @note AActor::StaticClass()->GetSuperClass() == UObject::StaticClass()
     */
    UClass* GetSuperClass() const
    {
        return SuperClass;
    }

    UObject* GetDefaultObject() const
    {
        if (!ClassDefaultObject)
        {
            const_cast<UClass*>(this)->CreateDefaultObject();
        }
        return ClassDefaultObject;
    }
    
    UObject* CreateObject() const
    {
        return ClassConstructor();
    }

    static TSet<UClass*>& GetClassRegistry() {
        static TSet<UClass*> Registry;
        return Registry;
    }

    static void RegisterUClass(UClass* Class) {
        GetClassRegistry().Add(Class);
    }

    // UClass Iterator
    static auto Begin() { return GetClassRegistry().begin(); }
    static auto End() { return GetClassRegistry().end(); }
    
protected:
    virtual UObject* CreateDefaultObject();

private:
    [[maybe_unused]]
    uint32 ClassSize;
    [[maybe_unused]]
    uint32 ClassAlignment;

    UClass* SuperClass = nullptr;

    ObjectCreator ClassConstructor;
    UObject* ClassDefaultObject = nullptr;
};
