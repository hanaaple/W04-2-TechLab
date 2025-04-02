#include "Engine/Source/Runtime/CoreUObject/UObject/Object.h"

#include "ObjectFactory.h"
#include "UClass.h"
#include "UObjectHash.h"


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

bool UObject::IsA(const UClass* SomeBase) const
{
    const UClass* ThisClass = GetClass();
    return ThisClass->IsChildOf(SomeBase);
}

void UObject::CopyPropertiesFrom(UObject* Source)
{
    // 기본 멤버들 복사
    ClassPrivate = Source->ClassPrivate;
    //OuterPrivate = FObjectFactory::DuplicateObject(Source->OuterPrivate);
}