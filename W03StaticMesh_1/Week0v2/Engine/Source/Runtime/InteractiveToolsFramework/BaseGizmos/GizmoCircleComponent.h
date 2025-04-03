#pragma once
#include "GizmoBaseComponent.h"

class UGizmoCircleComponent : public UGizmoBaseComponent
{
    DECLARE_CLASS(UGizmoCircleComponent, UGizmoBaseComponent)

public:
    UGizmoCircleComponent();
    virtual ~UGizmoCircleComponent() override;
    virtual bool IntersectsRay(const FVector& rayOrigin, const FVector& rayDir, float& dist);

    float GetInnerRadius() const { return inner; }
    void SetInnerRadius(float value) { inner = value; }
    // 가상 복사 함수: 기본 UObject 멤버를 복사합니다.
    void CopyPropertiesFrom(UObject* Source, TMap<UObject*, UObject*>& DupMap) override;
    void CopyPropertiesTo(UObject* Dest, TMap<UObject*, UObject*>& OutMap) override;

private:
    float inner = 1.0f;
};
