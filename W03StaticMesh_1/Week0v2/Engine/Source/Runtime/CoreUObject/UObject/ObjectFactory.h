#pragma once
#include "Casts.h"
#include "Object.h"
#include "UObjectHash.h"
#include "EngineStatics.h"
#include "UObjectArray.h"

class FObjectFactory
{
public:
     template<typename T>
     static T* ConstructObject()
     {
         uint32 id = UEngineStatics::GenUUID();
         FString Name = T::StaticClass()->GetName() + "_" + std::to_string(id);
    
         T* Obj = new T;  // TODO: FPlatformMemory::Malloc으로 변경, placement new 사용시 Free방법 생각하기
         Obj->ClassPrivate = T::StaticClass();
         Obj->NamePrivate = Name;
         Obj->UUID = id;
    
         GUObjectArray.AddObject(Obj);
    
         UE_LOG(LogLevel::Display, "Created New Object : %s", *Name);
         return Obj;
     }

    //template<typename T>
    //    requires std::derived_from<T, UObject>
    //static T* ConstructObject(FName InName = FName(TEXT("")), UObject* InOuter = nullptr)
    //{
    //    UClass* InClass = T::StaticClass();
    //    uint32 id = UEngineStatics::GenUUID();
    //    FString NewName = InClass->GetName() + "_" + std::to_string(id);
    //    
    //    T* Obj = nullptr;
    //    
    //    // InName(Default 이름)을 기반으로 HashMap에서 Object 찾음 
    //    T* CDO = Cast<T>(StaticFindObjectFastInternal(InClass, InOuter, InClass->GetDefaultObjectName(), true));
    //        
    //    if (CDO == nullptr)
    //    {
    //        //새 DefaultObejct 생성
    //        CDO = ConstructDefaultObject<T>();
    //    }

    //    Obj = new T;
    //    Obj->CopyPropertiesFrom(CDO);
    //    GUObjectArray.AddObject(Obj);
    //    Obj->UUID = id;
    //    if (InName != FName(TEXT("")))
    //    {
    //        Obj->NamePrivate = InName;
    //    }
    //    else
    //    {
    //        Obj->NamePrivate = NewName;
    //    }
	
    //    return Obj;
    //}
    
    template<typename T>
    static T* ConstructDefaultObject()
    {
        UClass* InClass = T::StaticClass();
        FName Name = InClass->GetDefaultObjectName();
        uint32 id = UEngineStatics::GenUUID();

        T* Obj = new T;  // TODO: FPlatformMemory::Malloc으로 변경, placement new 사용시 Free방법 생각하기
        Obj->ClassPrivate = InClass;
        Obj->NamePrivate = Name;
        Obj->UUID = id;

        GUObjectArray.AddObject(Obj);

        UE_LOG(LogLevel::Display, "Created New Object : %s", Name.ToString());
        return Obj;
    }
    
    // UObject 포인터를 키, 복제된 UObject 포인터를 값으로 저장합니다.
    using DuplicationMap = TMap<UObject*, UObject*>;

    template<typename T>
    static T* DuplicateObject(T* InDuplicated, const UClass* InClass = nullptr)
    {
        DuplicationMap DupMap;
        return DuplicateObject<T>(InDuplicated, InClass, DupMap);
    }
    
    template<typename T>
    static T* DuplicateObject(T* InDuplicated, const UClass* InClass, DuplicationMap& DupMap)
     {
         if (InDuplicated == nullptr)
             return nullptr;

         // 이미 복제된 객체가 있는지 확인
         auto Found = DupMap.Find(InDuplicated);
         if (Found != nullptr)
         {
             return Cast<T>(*Found);
         }

         if (InClass == nullptr)
         {
             InClass = T::StaticClass();
         }

         uint32 id = UEngineStatics::GenUUID();
         FString Name = InClass->GetName() + "__Duplicated" + std::to_string(id);
    
         // 새 객체 생성
         T* Obj = new T();
    
         // 복제 맵에 등록하여 재귀 호출 시 순환 참조를 방지
         DupMap[InDuplicated] = Obj;

         // 객체의 속성 복사 (DupMap을 전달)
         Obj->CopyPropertiesFrom(InDuplicated, DupMap);
         Obj->UUID = id;
         Obj->NamePrivate = Name;
         GUObjectArray.AddObject(Obj);

         return Obj;
     }
};
