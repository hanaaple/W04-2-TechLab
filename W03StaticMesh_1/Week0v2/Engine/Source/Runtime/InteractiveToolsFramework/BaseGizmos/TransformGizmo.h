#pragma once
#include "GameFramework/Actor.h"


class UStaticMeshComponent;
class UTransformGizmo : public AActor
{
    DECLARE_CLASS(UTransformGizmo, AActor)

public:
    UTransformGizmo();

    virtual void Tick(float DeltaTime) override;

    TArray<UStaticMeshComponent*>& GetArrowArr() { return ArrowArr; }
    TArray<UStaticMeshComponent*>& GetDiscArr() { return CircleArr; }
    TArray<UStaticMeshComponent*>& GetScaleArr() { return RectangleArr; }
    // 가상 복사 함수: 기본 UObject 멤버를 복사합니다.
    void CopyPropertiesFrom(UObject* Source) override;
private:
    TArray<UStaticMeshComponent*> ArrowArr;
    TArray<UStaticMeshComponent*> CircleArr;
    TArray<UStaticMeshComponent*> RectangleArr;
};
