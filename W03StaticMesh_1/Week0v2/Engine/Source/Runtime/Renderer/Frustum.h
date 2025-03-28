#pragma once
#include "Launch/Define.h"
struct FFrustumPlane
{
    FVector Normal;
    float D;

    void Normalize();
};

class FFrustum
{
public:
    FFrustum(const FMatrix& ViewMatrix, const FMatrix& ProjectionMatrix);

    bool IsBoxVisible(const FBoundingBox& Box) const;

private:
    FFrustumPlane Planes[6];

    void ExtractPlanes(const FMatrix& ViewProj);
};