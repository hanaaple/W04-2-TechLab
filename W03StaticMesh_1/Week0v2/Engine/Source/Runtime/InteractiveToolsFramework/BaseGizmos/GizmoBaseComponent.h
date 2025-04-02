#pragma once
#include "Components/StaticMeshComponent.h"


class UGizmoBaseComponent : public UStaticMeshComponent
{
    DECLARE_CLASS(UGizmoBaseComponent, UStaticMeshComponent)

public:
    enum GizmoType : uint8
    {
        ArrowX,
        ArrowY,
        ArrowZ,
        CircleX,
        CircleY,
        CircleZ,
        ScaleX,
        ScaleY,
        ScaleZ
    };
public:
    UGizmoBaseComponent() = default;

    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;
    virtual void TickComponent(float DeltaTime) override;

private:
    GizmoType gizmoType;

public:
    GizmoType GetGizmoType() const { return gizmoType; }
    void SetGizmoType(GizmoType _gizmoType) { gizmoType = _gizmoType; }
    // 가상 복사 함수: 기본 UObject 멤버를 복사합니다.
    void CopyPropertiesFrom(UObject* Source, TMap<UObject*, UObject*>& DupMap) override;
};
