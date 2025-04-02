#pragma once
#include "PrimitiveComponent.h"
#include "Define.h"
class UBillboardComponent;

class ULightComponentBase : public USceneComponent
{
    DECLARE_CLASS(ULightComponentBase, USceneComponent)

public:
    ULightComponentBase();
    virtual ~ULightComponentBase() override;

    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance);
    void InitializeLight();
    void SetColor(FVector4 newColor);
    FVector4 GetColor() const;
    float GetRadius() const;
    void SetRadius(float r);
    // 가상 복사 함수: 기본 UObject 멤버를 복사합니다.
    void CopyPropertiesFrom(UObject* Source) override;
private:
    FVector4 color;
    float radius;
    FBoundingBox AABB;
    UBillboardComponent* texture2D;
public:
    FBoundingBox GetBoundingBox() const {return AABB;}
    float GetRadius() {return radius;}
    FVector4 GetColor() {return color;}
    UBillboardComponent* GetTexture2D() const {return texture2D;}
};
