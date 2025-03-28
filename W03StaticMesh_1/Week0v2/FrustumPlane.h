#pragma once
#include "Launch/Define.h"

struct FFrustumPlane
{
    FVector Normal;
    float D;

    void Normalize()
    {
        float length = Normal.Magnitude();
        Normal = Normal.Normalize();
        D /= length;
    }
};

void ExtractFrustumPlanes(FFrustumPlane Planes[6], const FMatrix& ViewProjection)
{
    const FMatrix& m = ViewProjection;

    // Left
    Planes[0].Normal.x = m.M[0][3] + m.M[0][0];
    Planes[0].Normal.y = m.M[1][3] + m.M[1][0];
    Planes[0].Normal.z = m.M[2][3] + m.M[2][0];
    Planes[0].D = m.M[3][3] + m.M[3][0];

    // Right
    Planes[1].Normal.x = m.M[0][3] - m.M[0][0];
    Planes[1].Normal.y = m.M[1][3] - m.M[1][0];
    Planes[1].Normal.z = m.M[2][3] - m.M[2][0];
    Planes[1].D = m.M[3][3] - m.M[3][0];

    // Bottom
    Planes[2].Normal.x = m.M[0][3] + m.M[0][1];
    Planes[2].Normal.y = m.M[1][3] + m.M[1][1];
    Planes[2].Normal.z = m.M[2][3] + m.M[2][1];
    Planes[2].D = m.M[3][3] + m.M[3][1];

    // Top
    Planes[3].Normal.x = m.M[0][3] - m.M[0][1];
    Planes[3].Normal.y = m.M[1][3] - m.M[1][1];
    Planes[3].Normal.z = m.M[2][3] - m.M[2][1];
    Planes[3].D = m.M[3][3] - m.M[3][1];

    // Near
    Planes[4].Normal.x = m.M[0][3] + m.M[0][2];
    Planes[4].Normal.y = m.M[1][3] + m.M[1][2];
    Planes[4].Normal.z = m.M[2][3] + m.M[2][2];
    Planes[4].D = m.M[3][3] + m.M[3][2];

    // Far
    Planes[5].Normal.x = m.M[0][3] - m.M[0][2];
    Planes[5].Normal.y = m.M[1][3] - m.M[1][2];
    Planes[5].Normal.z = m.M[2][3] - m.M[2][2];
    Planes[5].D = m.M[3][3] - m.M[3][2];

    for (int i = 0; i < 6; ++i)
        Planes[i].Normalize();
}
