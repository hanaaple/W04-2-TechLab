#pragma once
#include "UText.h"

class UTextUUID : public UText
{
    DECLARE_CLASS(UTextUUID, UText)

public:
    UTextUUID();
    virtual ~UTextUUID() override;

    virtual int CheckRayIntersection(
        const FVector& rayOrigin,
        const FVector& rayDirection, float& pfNearHitDistance
    ) override;
    void SetUUID(uint32 UUID);
};
