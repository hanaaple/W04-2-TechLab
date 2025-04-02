// ReSharper disable CppClangTidyBugproneMacroParentheses
#pragma once
#include "UClass.h"

// name을 문자열화 해주는 매크로
#define INLINE_STRINGIFY(name) #name


// RTTI를 위한 클래스 매크로
#define DECLARE_CLASS(TClass, TSuperClass) \
private: \
    TClass(const TClass&) = delete; \
    TClass& operator=(const TClass&) = delete; \
    TClass(TClass&&) = delete; \
    TClass& operator=(TClass&&) = delete; \
public: \
    using Super = TSuperClass; \
    using ThisClass = TClass; \
    static UClass* StaticClass() { \
        static UClass ClassInfo{ TEXT(#TClass), static_cast<uint32>(sizeof(TClass)), static_cast<uint32>(alignof(TClass)), TSuperClass::StaticClass() }; \
        return &ClassInfo; \
    }

// Defines all bitwise operators for enum classes so it can be (mostly) used as a regular flags enum
#define ENUM_CLASS_FLAGS(Enum) \
inline           Enum& operator|=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs | (__underlying_type(Enum))Rhs); } \
inline           Enum& operator&=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs & (__underlying_type(Enum))Rhs); } \
inline           Enum& operator^=(Enum& Lhs, Enum Rhs) { return Lhs = (Enum)((__underlying_type(Enum))Lhs ^ (__underlying_type(Enum))Rhs); } \
inline constexpr Enum  operator| (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs | (__underlying_type(Enum))Rhs); } \
inline constexpr Enum  operator& (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs & (__underlying_type(Enum))Rhs); } \
inline constexpr Enum  operator^ (Enum  Lhs, Enum Rhs) { return (Enum)((__underlying_type(Enum))Lhs ^ (__underlying_type(Enum))Rhs); } \
inline constexpr bool  operator! (Enum  E)             { return !(__underlying_type(Enum))E; } \
inline constexpr Enum  operator~ (Enum  E)             { return (Enum)~(__underlying_type(Enum))E; }


// #define PROPERTY(Type, VarName, DefaultValue) \
// private: \
//     Type VarName DefaultValue; \
// public: \
//     Type Get##VarName() const { return VarName; } \
//     void Set##VarName(const Type& New##VarName) { VarName = New##VarName; }

// Getter & Setter 생성
#define PROPERTY(type, name) \
    void Set##name(const type& value) { name = value; } \
    type Get##name() const { return name; }
