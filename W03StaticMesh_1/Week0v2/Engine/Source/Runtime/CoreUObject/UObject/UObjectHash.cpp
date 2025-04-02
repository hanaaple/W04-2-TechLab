#include "UObjectHash.h"
#include <cassert>
#include "Object.h"
#include "UClass.h"
#include "UObjectArray.h"
#include "Container/Map.h"
#include "Container/MultiMap.h"
#include "Container/Set.h"
#include "Math/JungleMath.h"
#include "Math/MathUtility.h"

/**
 * 모든 UObject의 정보를 담고 있는 HashTable
 */
struct FUObjectHashTables
{
    static FUObjectHashTables& Get()
    {
        static FUObjectHashTables Singleton;
        return Singleton;
    }

    TMap<UClass*, TSet<UClass*>> ClassToChildListMap;
    TMap<UClass*, TSet<UObject*>> ClassToObjectListMap;

    TMap<int32, TSet<UObject*>> Hash;
    TMultiMap<int32, uint32> HashOuter;
    TMap<UObject*, TSet<UObject*>> ObjectOuterMap;

    FORCEINLINE void AddToHash(const int32 InHash, UObject* Object)
    {
        TSet<UObject*>& Bucket = Hash.FindOrAdd(InHash);
        Bucket.Add(Object);
    }

    FORCEINLINE int32 RemoveFromHash(int32 InHash, UObject* Object)
    {
        int32 NumRemoved = 0;
        TSet<UObject*>* Bucket = Hash.Find(InHash);
        if (Bucket)
        {
            NumRemoved = Bucket->Remove(Object);
            if (Bucket->Num() == 0)
            {
                Hash.Remove(InHash);
            }
        }
        return NumRemoved;
    }
};

/** Helper function that returns all the children of the specified class recursively */
template <typename ClassType, typename ArrayAllocator>
static void RecursivelyPopulateDerivedClasses(FUObjectHashTables& ThreadHash, const UClass* ParentClass, TArray<ClassType, ArrayAllocator>& OutAllDerivedClass)
{
    // Start search with the parent class at virtual index Num-1, then continue searching from index Num as things are added
    int32 SearchIndex = OutAllDerivedClass.Num() - 1;
    const UClass* SearchClass = ParentClass;

    while (true)
    {
        if (TSet<UClass*>* ChildSet = ThreadHash.ClassToChildListMap.Find(const_cast<UClass*>(SearchClass)))
        {
            OutAllDerivedClass.Reserve(OutAllDerivedClass.Num() + ChildSet->Num());
            for (UClass* ChildClass : *ChildSet)
            {
                OutAllDerivedClass.Add(ChildClass);
            }
        }

        // Now search at next index, if it has been filled in by code above
        ++SearchIndex;

        if (SearchIndex < OutAllDerivedClass.Num())
        {
            SearchClass = OutAllDerivedClass[SearchIndex];			
        }
        else
        {
            return;
        }
    }
}

void AddToClassMap(UObject* Object)
{
    assert(Object->GetClass());
    FUObjectHashTables& HashTable = FUObjectHashTables::Get();

    UClass* Class = Object->GetClass();
    HashTable.ClassToObjectListMap.FindOrAdd(Class).Add(Object);

    for (UClass* SuperClass = Class->GetSuperClass(); SuperClass;)
    {
        HashTable.ClassToChildListMap.FindOrAdd(SuperClass).Add(Class);
    
        Class = SuperClass;
        SuperClass = SuperClass->GetSuperClass();
    }
}

void RemoveFromClassMap(UObject* Object)
{
    assert(Object->GetClass());
    FUObjectHashTables& HashTable = FUObjectHashTables::Get();

    TSet<UObject*>& ObjectSet = HashTable.ClassToObjectListMap.FindOrAdd(Object->GetClass());
    const int32 NumRemoved = ObjectSet.Remove(Object);

    if (NumRemoved == 0)
    {
        HashTable.ClassToObjectListMap.Remove(Object->GetClass());
    }
}

void AddToOuterMap(UObject* Object)
{
    FUObjectHashTables& HashTable = FUObjectHashTables::Get();
    TSet<UObject*> Bucket = HashTable.ObjectOuterMap.FindOrAdd(Object->GetOuter());
    if (!Bucket.Contains(Object))
    {
        Bucket.Add(Object);
    }
}

void RemoveFromOuterMap(UObject* Object)
{
    FUObjectHashTables& HashTable = FUObjectHashTables::Get();

    const TSet<UObject*>& Bucket = HashTable.ObjectOuterMap.FindOrAdd(Object->GetOuter());

    if (!Bucket.Num())
    {
        HashTable.ObjectOuterMap.Remove(Object->GetOuter());
    }
}


static UObject* StaticFindObjectFastInternalThreadSafe(FUObjectHashTables& ThreadHash, const UClass* ObjectClass, const UObject* ObjectPackage, const FName ObjectName, const bool bExactClass, bool bAnyPackage)
{
    UObject* Result = nullptr;
    if (ObjectPackage != nullptr)
    {
        const std::intptr_t Outer = reinterpret_cast<std::intptr_t>(ObjectPackage);

        // Outer의 Hash
        const int32 Hash = ObjectName.GetComparisonIndex() + static_cast<int32>(Outer >> 6);
        FUObjectHashTables& ThreadHash = FUObjectHashTables::Get();

        const auto range = ThreadHash.HashOuter.EqualRange(Hash);
        for (auto it = range.first; it != range.second; ++it)
        {
            // Outer의 UUID
            const uint32 InternalIndex = it->second;
            UObject* Object = static_cast<UObject*>(GUObjectArray.GetObjectByUUID(InternalIndex));
            if (Object != nullptr)
            {
                    /* check that the name matches the name we're searching for */
                if (Object->GetFName() == ObjectName
                    /* check that the object has the correct Outer */
                    && Object->GetOuter() == ObjectPackage
                    /** If a class was specified, check that the object is of the correct class */
                    && (ObjectClass == nullptr || (bExactClass ? Object->GetClass() == ObjectClass : Object->IsA(ObjectClass))))
                {
                    Result = Object;
                }
            }
        }
    }
    else
    {
        // ObjectName이 경로 일 때? -> Innder로 ObjectName을 꺼냄
        const FName Inner = ExtractInnerFromFName(ObjectName);
        const int32 Hash = Inner.GetComparisonIndex();

        TSet<UObject*>* Bucket = ThreadHash.Hash.Find(Hash);
        for (auto It = Bucket->begin(); It != Bucket->end(); ++It)
        {
            UObject* Object = *It;

            if (Object != nullptr)
            {
                /* check that the name matches the name we're searching for */
                if (Object->GetFName() == ObjectName
                    /* check that the object has the correct Outer */
                    && Object->GetOuter() == ObjectPackage
                    /** If a class was specified, check that the object is of the correct class */
                    && (ObjectClass == nullptr || (bExactClass ? Object->GetClass() == ObjectClass : Object->IsA(ObjectClass))))
                {
                    Result = Object;
                }
            }
        }
    }
    
    return Result;
}

UObject* StaticFindObjectFastInternal(const UClass* Class, const UObject* InOuter, const FName InName, const bool ExactClass)
{
    UObject* Result = nullptr;
    FUObjectHashTables& ThreadHash = FUObjectHashTables::Get();
    Result = StaticFindObjectFastInternalThreadSafe(ThreadHash, Class, InOuter, InName, ExactClass, /*bAnyPackage =*/ false);

    return Result;
}

FName ExtractInnerFromFName(const FName& InPath)
{
    // FName을 문자열로 변환합니다.
    FString PathStr = InPath.ToString();
    
    // '.'와 ':' 중 마지막 구분자 위치를 찾습니다.
    // (FString의 FindLastChar를 사용하거나, rfind와 유사한 로직을 구현할 수 있습니다.)
    const int32 DotIndex = PathStr.FindLastChar(TEXT('.'));
    const int32 ColonIndex = PathStr.FindLastChar(TEXT(':'));
    
    // 두 구분자 중 더 큰 인덱스가 마지막 구분자입니다.
    const int32 LastDelimiterIndex = FMath::Max(DotIndex, ColonIndex);
    
    // 구분자가 없으면 전체 문자열이 Inner가 됩니다.
    if (LastDelimiterIndex == INDEX_NONE)
    {
        return InPath;
    }
    
    // 마지막 구분자 이후의 부분 문자열을 Inner로 사용합니다.
    const FString InnerStr = PathStr.Mid(LastDelimiterIndex + 1);
    return FName(InnerStr);
}

void GetObjectsOfClass(const UClass* ClassToLookFor, TArray<UObject*>& Results, bool bIncludeDerivedClasses)
{
    // Most classes searched for have around 10 subclasses, some have hundreds
    TArray<const UClass*> ClassesToSearch;
    ClassesToSearch.Add(ClassToLookFor);

    FUObjectHashTables& ThreadHash = FUObjectHashTables::Get();

    if (bIncludeDerivedClasses) 
    {
        RecursivelyPopulateDerivedClasses(ThreadHash, ClassToLookFor, ClassesToSearch);
    }


    for (const UClass* SearchClass : ClassesToSearch)
    {
        if (TSet<UObject*>* List = ThreadHash.ClassToObjectListMap.Find(const_cast<UClass*>(SearchClass)))
        {
            for (const auto& Object : *List)
            {
                Results.Add(Object);
            }
        }
    }
}

void HashObject(UObject* Object)
{
    const FName Name = Object->GetFName();
    if (Name != FName(TEXT("")))
    {
        int32 Hash = 0;
        
        FUObjectHashTables& ThreadHash = FUObjectHashTables::Get();
        Hash = Name.GetComparisonIndex();

        ThreadHash.AddToHash(Hash, Object);

        if (const std::intptr_t Outer = reinterpret_cast<std::intptr_t>(Object->GetOuter()))
        {
            Hash = Name.GetComparisonIndex() + static_cast<int32>(Outer >> 6);
            ThreadHash.HashOuter.Add(Hash, Object->GetUUID());

            AddToOuterMap(Object);
        }

        AddToClassMap(Object);
    }
}

void UnHashObject(UObject* Object)
{
    const FName Name = Object->GetFName();
    if (Name != FName(TEXT("")))
    {
        int32 Hash = 0;
        uint32 NumRemoved = 0;
        
        FUObjectHashTables& ThreadHash = FUObjectHashTables::Get();
        Hash = Name.GetComparisonIndex();
        NumRemoved = ThreadHash.RemoveFromHash(Hash, Object);

        if (const std::intptr_t Outer = reinterpret_cast<std::intptr_t>(Object->GetOuter()))
        {
            Hash = Name.GetComparisonIndex() + static_cast<int32>(Outer >> 6);
            NumRemoved = ThreadHash.HashOuter.RemoveSingle(Hash, Object->GetUUID());

            RemoveFromOuterMap(Object);
        }

        RemoveFromClassMap(Object);
    }
}
