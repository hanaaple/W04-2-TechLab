#pragma once
#include "UText.h"

class UTextUUID : public UText
{
    DECLARE_CLASS(UTextUUID, UText)

public:
    UTextUUID();
    virtual ~UTextUUID() override;

    virtual int CheckRayIntersection(
        FVector& rayOrigin,
        FVector& rayDirection, float& pfNearHitDistance
    ) override;
    void SetUUID(uint32 UUID);
    // 가상 복사 함수: 기본 UObject 멤버를 복사합니다.
    void CopyPropertiesFrom(UObject* Source) override;
};
