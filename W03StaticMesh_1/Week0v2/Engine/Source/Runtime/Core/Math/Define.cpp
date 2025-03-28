#include "Define.h"

// 단위 행렬 정의.
const FMatrix FMatrix::Identity = FMatrix(
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
);

inline FMatrix::FMatrix() noexcept {
    M[0][0] = 0.0f; M[1][0] = 0.0f; M[2][0] = 0.0f; M[3][0] = 0.0f;
    M[0][1] = 0.0f; M[1][1] = 0.0f; M[2][1] = 0.0f; M[3][1] = 0.0f;
    M[0][2] = 0.0f; M[1][2] = 0.0f; M[2][2] = 0.0f; M[3][2] = 0.0f;
    M[0][3] = 0.0f; M[1][3] = 0.0f; M[2][3] = 0.0f; M[3][3] = 0.0f;
}

inline constexpr FMatrix::FMatrix(
    float m00, float m10, float m20, float m30, 
    float m01, float m11, float m21, float m31, 
    float m02, float m12, float m22, float m32, 
    float m03, float m13, float m23, float m33
) noexcept {
    //M[0][0] = m00; M[0][1] = m10; M[0][2] = m20; M[0][3] = m30;
    //M[1][0] = m01; M[1][1] = m11; M[1][2] = m21; M[1][3] = m31;
    //M[2][0] = m02; M[2][1] = m12; M[2][2] = m22; M[2][3] = m32;
    //M[3][0] = m03; M[3][1] = m13; M[3][2] = m23; M[3][3] = m33;
    M[0][0] = m00; M[1][0] = m10; M[2][0] = m20; M[3][0] = m30;
    M[0][1] = m01; M[1][1] = m11; M[2][1] = m21; M[3][1] = m31;
    M[0][2] = m02; M[1][2] = m12; M[2][2] = m22; M[3][2] = m32;
    M[0][3] = m03; M[1][3] = m13; M[2][3] = m23; M[3][3] = m33;
}

// 행렬 덧셈.
FMatrix FMatrix::operator+(const FMatrix& Other) const {
#ifdef _USING_SIMD
    using namespace DirectX;
    XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M[0]));
    XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M[1]));
    XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M[2]));
    XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M[3]));

    const XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&Other.M[0]));
    const XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&Other.M[1]));
    const XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&Other.M[2]));
    const XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&Other.M[3]));

    x1 = XMVectorAdd(x1, y1);
    x2 = XMVectorAdd(x2, y2);
    x3 = XMVectorAdd(x3, y3);
    x4 = XMVectorAdd(x4, y4);

    FMatrix m;

    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&m.M[0]), x1);
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&m.M[1]), x2);
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&m.M[2]), x3);
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&m.M[3]), x4);
    return m;
#else
    FMatrix Result;
    for (int32 i = 0; i < 4; i++)
        for (int32 j = 0; j < 4; j++)
            Result.M[i][j] = M[i][j] + Other.M[i][j];
    return Result;
#endif
}

// 행렬 뺄셈.
FMatrix FMatrix::operator-(const FMatrix& Other) const {
#ifdef _USING_SIMD
    using namespace DirectX;
    XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M[0]));
    XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M[1]));
    XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M[2]));
    XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M[3]));

    const XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&Other.M[0]));
    const XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&Other.M[1]));
    const XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&Other.M[2]));
    const XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&Other.M[3]));

    x1 = XMVectorSubtract(x1, y1);
    x2 = XMVectorSubtract(x2, y2);
    x3 = XMVectorSubtract(x3, y3);
    x4 = XMVectorSubtract(x4, y4);

    FMatrix m;

    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&m.M[0]), x1);
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&m.M[1]), x2);
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&m.M[2]), x3);
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&m.M[3]), x4);
    return m;
#else
    FMatrix Result;
    for ( int32 i = 0; i < 4; i++ )
        for ( int32 j = 0; j < 4; j++ )
            Result.M[i][j] = M[i][j] - Other.M[i][j];
    return Result;
#endif
}

// 행렬 곱셈.
FMatrix FMatrix::operator*(const FMatrix& Other) const {
#ifdef _USING_SIMD
    using namespace DirectX;
    const XMMATRIX M1 = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(M));
    const XMMATRIX M2 = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(Other.M));
    const XMMATRIX X = XMMatrixMultiply(M1, M2);
    FMatrix res;
    XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(res.M), X);
    return res;
#else
    FMatrix Result = {};
    for (int32 i = 0; i < 4; i++)
        for (int32 j = 0; j < 4; j++)
            for (int32 k = 0; k < 4; k++)
                Result.M[i][j] += M[i][k] * Other.M[k][j];
    return Result;
#endif
}

// 스칼라 곱셈.
FMatrix FMatrix::operator*(float Scalar) const {
#ifdef _USING_SIMD
    using namespace DirectX;
    XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M[0]));
    XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M[1]));
    XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M[2]));
    XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M[3]));

    x1 = XMVectorScale(x1, Scalar);
    x2 = XMVectorScale(x2, Scalar);
    x3 = XMVectorScale(x3, Scalar);
    x4 = XMVectorScale(x4, Scalar);

    FMatrix m;
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&m.M[0]), x1);
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&m.M[1]), x2);
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&m.M[2]), x3);
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&m.M[3]), x4);
    return m;
#else
    FMatrix Result;
    for ( int32 i = 0; i < 4; i++ )
        for ( int32 j = 0; j < 4; j++ )
            Result.M[i][j] = M[i][j] * Scalar;
    return Result;
#endif
}

// 스칼라 나눗셈.
FMatrix FMatrix::operator/(float Scalar) const {
#ifdef _USING_SIMD
    using namespace DirectX;
    assert(Scalar != 0.f);
    const float rs = 1.f / Scalar;

    XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M[0]));
    XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M[1]));
    XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M[2]));
    XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M[3]));

    x1 = XMVectorScale(x1, rs);
    x2 = XMVectorScale(x2, rs);
    x3 = XMVectorScale(x3, rs);
    x4 = XMVectorScale(x4, rs);

    FMatrix m;
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&m.M[0]), x1);
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&m.M[1]), x2);
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&m.M[2]), x3);
    XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&m.M[3]), x4);
    return m;
#else
    FMatrix Result;
    for (int32 i = 0; i < 4; i++)
        for (int32 j = 0; j < 4; j++)
            Result.M[i][j] = M[i][j] / Scalar;
    return Result;
#endif
}

float* FMatrix::operator[](int row) {
    return M[row];
}

const float* FMatrix::operator[](int row) const
{
    return M[row];
}

// 전치 행렬.
FMatrix FMatrix::Transpose(const FMatrix& Mat) {
#ifdef _USING_SIMD
    using namespace DirectX;
    const XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(Mat.M));
    FMatrix res;
    XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(res.M), XMMatrixTranspose(M));
    return res;
#else
    FMatrix Result;
    for (int32 i = 0; i < 4; i++)
        for (int32 j = 0; j < 4; j++)
            Result.M[i][j] = Mat.M[j][i];
    return Result;
#endif
}

// 행렬식 계산.
float FMatrix::Determinant(const FMatrix& Mat) {
#ifdef _USING_SIMD
    using namespace DirectX;
    const XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(Mat.M));
    return XMVectorGetX(XMMatrixDeterminant(M));
#else
    // 행렬식 계산 (라플라스 전개, 4x4 행렬)
    float det = 0.0f;
    for (int32 i = 0; i < 4; i++) {
        float subMat[3][3];
        for (int32 j = 1; j < 4; j++) {
            int32 colIndex = 0;
            for (int32 k = 0; k < 4; k++) {
                if (k == i) continue;
                subMat[j - 1][colIndex] = Mat.M[j][k];
                colIndex++;
            }
        }
        float minorDet =
            subMat[0][0] * (subMat[1][1] * subMat[2][2] - subMat[1][2] * subMat[2][1]) -
            subMat[0][1] * (subMat[1][0] * subMat[2][2] - subMat[1][2] * subMat[2][0]) +
            subMat[0][2] * (subMat[1][0] * subMat[2][1] - subMat[1][1] * subMat[2][0]);
        det += (i % 2 == 0 ? 1 : -1) * Mat.M[0][i] * minorDet;
    }
    return det;
#endif
}

// 역행렬.
FMatrix FMatrix::Inverse(const FMatrix& Mat) {
#ifdef _USING_SIMD
    using namespace DirectX;
    const XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(Mat.M));
    FMatrix res;
    XMVECTOR det;
    XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(res.M), XMMatrixInverse(&det, M));
    return res;
#else
    // 역행렬 (가우스-조던 소거법)
    float det = Determinant(Mat);
    if (fabs(det) < 1e-6) {
        return Identity;
    }

    FMatrix Inv;
    float invDet = 1.0f / det;

    // 여인수 행렬 계산 후 전치하여 역행렬 계산
    for (int32 i = 0; i < 4; i++) {
        for (int32 j = 0; j < 4; j++) {
            float subMat[3][3];
            int32 subRow = 0;
            for (int32 r = 0; r < 4; r++) {
                if (r == i) continue;
                int32 subCol = 0;
                for (int32 c = 0; c < 4; c++) {
                    if (c == j) continue;
                    subMat[subRow][subCol] = Mat.M[r][c];
                    subCol++;
                }
                subRow++;
            }
            float minorDet =
                subMat[0][0] * (subMat[1][1] * subMat[2][2] - subMat[1][2] * subMat[2][1]) -
                subMat[0][1] * (subMat[1][0] * subMat[2][2] - subMat[1][2] * subMat[2][0]) +
                subMat[0][2] * (subMat[1][0] * subMat[2][1] - subMat[1][1] * subMat[2][0]);

            Inv.M[j][i] = ((i + j) % 2 == 0 ? 1 : -1) * minorDet * invDet;
        }
    }
    return Inv;
#endif
}

FMatrix FMatrix::CreateRotation(float roll, float pitch, float yaw)
{
    float radRoll = roll * (3.14159265359f / 180.0f);
    float radPitch = pitch * (3.14159265359f / 180.0f);
    float radYaw = yaw * (3.14159265359f / 180.0f);

    float cosRoll = cos(radRoll), sinRoll = sin(radRoll);
    float cosPitch = cos(radPitch), sinPitch = sin(radPitch);
    float cosYaw = cos(radYaw), sinYaw = sin(radYaw);

    //// Z축 (Yaw) 회전
    //FMatrix rotationZ = { {
    //    { cosYaw, sinYaw, 0, 0 },
    //    { -sinYaw, cosYaw, 0, 0 },
    //    { 0, 0, 1, 0 },
    //    { 0, 0, 0, 1 }
    //} };

    //// Y축 (Pitch) 회전
    //FMatrix rotationY = { {
    //    { cosPitch, 0, -sinPitch, 0 },
    //    { 0, 1, 0, 0 },
    //    { sinPitch, 0, cosPitch, 0 },
    //    { 0, 0, 0, 1 }
    //} };
    // 
    // //// Y축 (Pitch) 회전
    //FMatrix rotationY = { {
    //    { cosPitch, 0, -sinPitch, 0 },
    //    { 0, 1, 0, 0 },
    //    { sinPitch, 0, cosPitch, 0 },
    //    { 0, 0, 0, 1 }
    //} };

    FMatrix rotationZ = FMatrix(
        cosYaw, sinYaw, 0, 0,
        -sinYaw, cosYaw, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );

    // Y축 (Pitch) 회전
    FMatrix rotationY = FMatrix(
        cosPitch, 0, -sinPitch, 0,
        0, 1, 0, 0,
        sinPitch, 0, cosPitch, 0,
        0, 0, 0, 1
    );

    // X축 (Roll) 회전
    FMatrix rotationX = FMatrix(
        1, 0, 0, 0,
        0, cosRoll, sinRoll, 0,
        0, -sinRoll, cosRoll, 0,
        0, 0, 0, 1
    );

    // DirectX 표준 순서: Z(Yaw) → Y(Pitch) → X(Roll)  
    return rotationX * rotationY * rotationZ;  // 이렇게 하면  오른쪽 부터 적용됨
}


// 스케일 행렬 생성
FMatrix FMatrix::CreateScale(float scaleX, float scaleY, float scaleZ)
{
    return FMatrix(
        scaleX, 0, 0, 0,
        0, scaleY, 0, 0,
        0, 0, scaleZ, 0,
        0, 0, 0, 1
    );
}

FMatrix FMatrix::CreateTranslationMatrix(const FVector& position)
{
    FMatrix translationMatrix = FMatrix::Identity;
    translationMatrix.M[3][0] = position.x;
    translationMatrix.M[3][1] = position.y;
    translationMatrix.M[3][2] = position.z;
    return translationMatrix;
}

FVector FMatrix::TransformVector(const FVector& v, const FMatrix& m)
{
    FVector result;

    // 4x4 행렬을 사용하여 벡터 변환 (W = 0으로 가정, 방향 벡터)
    result.x = v.x * m.M[0][0] + v.y * m.M[1][0] + v.z * m.M[2][0] + 0.0f * m.M[3][0];
    result.y = v.x * m.M[0][1] + v.y * m.M[1][1] + v.z * m.M[2][1] + 0.0f * m.M[3][1];
    result.z = v.x * m.M[0][2] + v.y * m.M[1][2] + v.z * m.M[2][2] + 0.0f * m.M[3][2];


    return result;
}

// FVector4를 변환하는 함수
FVector4 FMatrix::TransformVector(const FVector4& v, const FMatrix& m)
{
    FVector4 result;
    result.x = v.x * m.M[0][0] + v.y * m.M[1][0] + v.z * m.M[2][0] + v.a * m.M[3][0];
    result.y = v.x * m.M[0][1] + v.y * m.M[1][1] + v.z * m.M[2][1] + v.a * m.M[3][1];
    result.z = v.x * m.M[0][2] + v.y * m.M[1][2] + v.z * m.M[2][2] + v.a * m.M[3][2];
    result.a = v.x * m.M[0][3] + v.y * m.M[1][3] + v.z * m.M[2][3] + v.a * m.M[3][3];
    return result;
}


