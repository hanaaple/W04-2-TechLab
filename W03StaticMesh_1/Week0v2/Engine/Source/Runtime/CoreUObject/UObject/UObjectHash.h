#pragma once
#include "ObjectMacros.h"
#include "Container/Array.h"

class FName;
class UObject;
class UClass;

/**
 * ClassToLookFor와 일치하는 UObject를 반환합니다.
 * @param ClassToLookFor 반환할 Object의 Class정보
 * @param Results ClassToLookFor와 일치하는 모든 Objects가 담길 목록
 * @param bIncludeDerivedClasses ClassToLookFor의 파생 클래스까지 찾을지 여부
 */
void GetObjectsOfClass(const UClass* ClassToLookFor, TArray<UObject*>& Results, bool bIncludeDerivedClasses);

void HashObject(UObject* Object);

void UnHashObject(UObject* Object);

/** FUObjectHashTables에 Object의 정보를 저장합니다. */
void AddToClassMap(UObject* Object);

/** FUObjectHashTables에 저장된 Object정보를 제거합니다. */
void RemoveFromClassMap(UObject* Object);

void AddToOuterMap(UObject* Object);

void RemoveFromOuterMap(UObject* Object);

UObject* StaticFindObjectFastInternal(const UClass* Class, const UObject* InOuter, FName InName, bool ExactClass = false);

FName ExtractInnerFromFName(const FName& InPath);