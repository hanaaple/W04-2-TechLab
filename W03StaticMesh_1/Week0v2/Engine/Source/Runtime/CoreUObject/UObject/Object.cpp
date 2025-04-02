#include "Engine/Source/Runtime/CoreUObject/UObject/Object.h"
#include "Define.h"

#include "ObjectFactory.h"
#include "UClass.h"
#include "UObjectHash.h"
#include "Core/Container/String.h"
#include "UnrealEd/Editor/EditorEngine.h"

UClass* UObject::StaticClass()
{
    static UClass ClassInfo{TEXT("UObject"), sizeof(UObject), alignof(UObject), nullptr};
    return &ClassInfo;
}

UObject::UObject()
    : UUID(0)
    // TODO: Object를 생성할 때 직접 설정하기
    , InternalIndex(std::numeric_limits<uint32>::max())
    , NamePrivate(TEXT("None"))
{
}

void* UObject::operator new(size_t size)
{
    UE_LOG(LogLevel::Display, "UObject Created : %d", size);

    void* RawMemory = FPlatformMemory::Malloc<EAT_Object>(size);
    UE_LOG(
        LogLevel::Display,
        "TotalAllocationBytes : %d, TotalAllocationCount : %d",
        FPlatformMemory::GetAllocationBytes<EAT_Object>(),
        FPlatformMemory::GetAllocationCount<EAT_Object>()
    );
    return RawMemory;
}

void UObject::operator delete(void* ptr, size_t size)
{
    UE_LOG(LogLevel::Display, "UObject Deleted : %d", size);
    FPlatformMemory::Free<EAT_Object>(ptr, size);
}

UWorld* UObject::GetWorld()
{
    return GEngineLoop.GetWorld();
}

FEditorEngine& UObject::GetEngine()
{
    return GEngineLoop;
}

bool UObject::IsA(const UClass* SomeBase) const
{
    const UClass* ThisClass = GetClass();
    return ThisClass->IsChildOf(SomeBase);
}

UObject* UObject::Duplicate()
{
    return FObjectFactory::DuplicateObject(this, GetClass());
}

FString UObject::GetName() const
{
     return NamePrivate.ToString(); 
}

FName UObject::GetFName() const
{
     return NamePrivate; 
}
uint32 UObject::GetUUID() const { return UUID; }
uint32 UObject::GetInternalIndex() const { return InternalIndex; }
UClass* UObject::GetClass() const { return ClassPrivate; }
FVector4 UObject::EncodeUUID() const {
    FVector4 result;

    result.x = UUID % 0xFF;
    result.y = UUID >> 8 & 0xFF;
    result.z = UUID >> 16 & 0xFF;
    result.a = UUID >> 24 & 0xFF;

    return result;
}