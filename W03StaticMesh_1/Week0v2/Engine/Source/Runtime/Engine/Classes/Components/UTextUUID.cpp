#include "UTextUUID.h"

UTextUUID::UTextUUID()
{
    SetScale(FVector(0.1f, 0.25f, 0.25f));
    SetLocation(FVector(0.0f, 0.0f, -0.5f));
}

UTextUUID::~UTextUUID()
{
}

int UTextUUID::CheckRayIntersection(const FVector& rayOrigin, const FVector& rayDirection, float& pfNearHitDistance)
{
    return 0;
}

void UTextUUID::SetUUID(uint32 UUID)
{
    SetText(std::to_wstring(UUID));
}


