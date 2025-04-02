#include "UClass.h"
#include <cassert>

#include "ObjectFactory.h"
#include "ObjectMacros.h"


UClass::UClass(const char* InClassName, uint32 InClassSize, uint32 InAlignment, UClass* InSuperClass)
    : ClassSize(InClassSize)
    , ClassAlignment(InAlignment)
    , SuperClass(InSuperClass)
{
    NamePrivate = InClassName;
}

FName UClass::GetDefaultObjectName() const
{
    const FString name = GetName();
    return FName(TEXT("Default__") + name);
}

bool UClass::IsChildOf(const UClass* SomeBase) const
{
    assert(this);
    if (!SomeBase) return false;

    // Super의 Super를 반복하면서 
    for (const UClass* TempClass = this; TempClass; TempClass=TempClass->GetSuperClass())
    {
        if (TempClass == SomeBase)
        {
            return true;
        }
    }
    return false;
}

void UClass::CopyPropertiesFrom(UObject* Source)
{
    Super::CopyPropertiesFrom(Source);
}

UObject* UClass::CreateDefaultObject()
{
    if (ClassDefaultObject == nullptr)
    {
        UClass* ParentClass = GetSuperClass();
        UObject* ParentDefaultObject = nullptr;
        if (ParentClass != nullptr)
        {
            ParentDefaultObject =  ParentClass->CreateDefaultObject();
        }

        if ((ParentDefaultObject != nullptr) && (this == UObject::StaticClass()))
        {
            if (ClassDefaultObject == nullptr)
            {
                // 부모의 CDO를 현재 클래스의 기본 객체로 설정
                ClassDefaultObject = ParentDefaultObject;
            }
        }
        else
        {
            // 일반적인 경우에는 새로 CDO를 생성합니다.
            // ClassDefaultObject = FObjectFactory::ConstructObject<UObject>();
            ClassDefaultObject = FObjectFactory::ConstructDefaultObject<UObject>();
            ClassDefaultObject->ClassPrivate = this;
            ClassDefaultObject->NamePrivate = FName(TEXT("Default__") + FString(GetName()));
            // 추가적인 초기화 작업(프로퍼티 기본값 설정, 컴포넌트 생성 등)이 필요하다면 여기에 추가
        }
    }

    return ClassDefaultObject;
}
