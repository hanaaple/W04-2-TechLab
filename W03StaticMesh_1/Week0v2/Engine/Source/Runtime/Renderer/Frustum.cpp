#include "Frustum.h"

void FFrustumPlane::Normalize()
{
    float length = Normal.Magnitude();
    Normal = Normal.Normalize();
    D /= length;
}

FFrustum::FFrustum(const FMatrix& ViewMatrix, const FMatrix& ProjectionMatrix)
{
    FMatrix ViewProj = ViewMatrix * ProjectionMatrix;
    ExtractPlanes(ViewProj);
}

bool FFrustum::IsBoxVisible(const FBoundingBox& Box) const
{
    for (int i = 0; i < 6; ++i)
    {
        const FFrustumPlane& Plane = Planes[i];

        FVector PositiveVertex(
            Plane.Normal.x >= 0 ? Box.max.x : Box.min.x,
            Plane.Normal.y >= 0 ? Box.max.y : Box.min.y,
            Plane.Normal.z >= 0 ? Box.max.z : Box.min.z
        );

        if (Plane.Normal.Dot(PositiveVertex) + Plane.D < 0)
        {
            return false;
        }
    }
    return true;
}

void FFrustum::ExtractPlanes(const FMatrix& ViewProj)
{
    const FMatrix& m = ViewProj;

    Planes[0].Normal.x = m.M[0][3] + m.M[0][0];
    Planes[0].Normal.y = m.M[1][3] + m.M[1][0];
    Planes[0].Normal.z = m.M[2][3] + m.M[2][0];
    Planes[0].D = m.M[3][3] + m.M[3][0];

    Planes[1].Normal.x = m.M[0][3] - m.M[0][0];
    Planes[1].Normal.y = m.M[1][3] - m.M[1][0];
    Planes[1].Normal.z = m.M[2][3] - m.M[2][0];
    Planes[1].D = m.M[3][3] - m.M[3][0];

    Planes[2].Normal.x = m.M[0][3] + m.M[0][1];
    Planes[2].Normal.y = m.M[1][3] + m.M[1][1];
    Planes[2].Normal.z = m.M[2][3] + m.M[2][1];
    Planes[2].D = m.M[3][3] + m.M[3][1];

    Planes[3].Normal.x = m.M[0][3] - m.M[0][1];
    Planes[3].Normal.y = m.M[1][3] - m.M[1][1];
    Planes[3].Normal.z = m.M[2][3] - m.M[2][1];
    Planes[3].D = m.M[3][3] - m.M[3][1];

    Planes[4].Normal.x = m.M[0][3] + m.M[0][2];
    Planes[4].Normal.y = m.M[1][3] + m.M[1][2];
    Planes[4].Normal.z = m.M[2][3] + m.M[2][2];
    Planes[4].D = m.M[3][3] + m.M[3][2];

    Planes[5].Normal.x = m.M[0][3] - m.M[0][2];
    Planes[5].Normal.y = m.M[1][3] - m.M[1][2];
    Planes[5].Normal.z = m.M[2][3] - m.M[2][2];
    Planes[5].D = m.M[3][3] - m.M[3][2];

    for (int i = 0; i < 6; ++i)
    {
        Planes[i].Normalize();
    }
}
