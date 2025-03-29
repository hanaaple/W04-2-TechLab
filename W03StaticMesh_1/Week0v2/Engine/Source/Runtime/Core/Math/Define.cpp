#include "Define.h"

#include "JungleMath.h"
#include "MathUtility.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Components/PrimitiveComponent.h"

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
    M[0][0] = m00; M[1][0] = m10; M[2][0] = m20; M[3][0] = m30;
    M[0][1] = m01; M[1][1] = m11; M[2][1] = m21; M[3][1] = m31;
    M[0][2] = m02; M[1][2] = m12; M[2][2] = m22; M[3][2] = m32;
    M[0][3] = m03; M[1][3] = m13; M[2][3] = m23; M[3][3] = m33;
}

// 행렬 덧셈.
FMatrix FMatrix::operator+(const FMatrix& Other) const {
#if defined(__AVX2__)
    FMatrix m;
    m._rowin256[0] = _mm256_add_ps(_rowin256[0], Other._rowin256[0]);
    m._rowin256[1] = _mm256_add_ps(_rowin256[1], Other._rowin256[1]);
    return m;
#elif defined(_XM_SSE_INTRINSICS_)
    FMatrix m;
    m.row[0] = _mm_add_ps(row[0], Other.row[0]);
    m.row[1] = _mm_add_ps(row[1], Other.row[1]);
    m.row[2] = _mm_add_ps(row[2], Other.row[2]);
    m.row[3] = _mm_add_ps(row[3], Other.row[3]);
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
#if defined(__AVX2__)
    FMatrix m;
    m._rowin256[0] = _mm256_sub_ps(_rowin256[0], Other._rowin256[0]);
    m._rowin256[1] = _mm256_sub_ps(_rowin256[1], Other._rowin256[1]);
    return m;
#elif defined(_XM_SSE_INTRINSICS_)
    FMatrix m;
    m.row[0] = _mm_sub_ps(row[0], Other.row[0]);
    m.row[1] = _mm_sub_ps(row[1], Other.row[1]);
    m.row[2] = _mm_sub_ps(row[2], Other.row[2]);
    m.row[3] = _mm_sub_ps(row[3], Other.row[3]);
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
#if defined(_XM_SSE_INTRINSICS_)
    FMatrix R;

    // B를 column 단위로 분해
    __m128 B0 = Other.row[0];
    __m128 B1 = Other.row[1];
    __m128 B2 = Other.row[2];
    __m128 B3 = Other.row[3];

    // A의 row들과 B의 columns 내적
    for ( int i = 0; i < 4; ++i ) {
        __m128 r = row[i];
        R.row[i] = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(r, r, _MM_SHUFFLE(0, 0, 0, 0)), B0),
                _mm_mul_ps(_mm_shuffle_ps(r, r, _MM_SHUFFLE(1, 1, 1, 1)), B1)
            ),
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(r, r, _MM_SHUFFLE(2, 2, 2, 2)), B2),
                _mm_mul_ps(_mm_shuffle_ps(r, r, _MM_SHUFFLE(3, 3, 3, 3)), B3)
            )
        );
    }
    return R;
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

#if defined(__AVX2__)
    FMatrix R;
    __m256 s = _mm256_set1_ps(Scalar);
    R._rowin256[0] = _mm256_mul_ps(_rowin256[0], s);
    R._rowin256[1] = _mm256_mul_ps(_rowin256[0], s);
    return R;
#elif defined(_XM_SSE_INTRINSICS_)
    FMatrix m;
    __m128 s = _mm_set_ps1(Scalar);
    m.row[0] = _mm_mul_ps(row[0], s);
    m.row[1] = _mm_mul_ps(row[1], s);
    m.row[2] = _mm_mul_ps(row[2], s);
    m.row[3] = _mm_mul_ps(row[3], s);
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
#if defined(__AVX2__)
    assert(Scalar != 0);
    const float RS = 1.f / Scalar;
    __m256 rs = _mm256_set1_ps(RS);
    FMatrix m;
    m._rowin256[0] = _mm256_mul_ps(_rowin256[0], rs);
    m._rowin256[1] = _mm256_mul_ps(_rowin256[1], rs);
    return m;
#elif defined(_XM_SSE_INTRINSICS_)
    assert(Scalar != 0);
    const float RS = 1.f / Scalar;
    __m128 rs = _mm_set_ps1(RS);
    FMatrix m;
    m.row[0] = _mm_mul_ps(row[0], rs);
    m.row[1] = _mm_mul_ps(row[1], rs);
    m.row[2] = _mm_mul_ps(row[2], rs);
    m.row[3] = _mm_mul_ps(row[3], rs);
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
#if defined(_XM_SSE_INTRINSICS_)
    FMatrix temp;
    // x.x, x.y, y.x, y.y
    temp.row[0] = _mm_shuffle_ps(Mat.row[0], Mat.row[1], _MM_SHUFFLE(1, 0, 1, 0));
    // x.z, x.w, y.z, y.w
    temp.row[2] = _mm_shuffle_ps(Mat.row[0], Mat.row[1], _MM_SHUFFLE(3, 2, 3, 2));
    // z.x, z.y, w.x, w.y
    temp.row[1] = _mm_shuffle_ps(Mat.row[2], Mat.row[3], _MM_SHUFFLE(1, 0, 1, 0));
    // z.z, z.w, w.z, w.w
    temp.row[3] = _mm_shuffle_ps(Mat.row[2], Mat.row[3], _MM_SHUFFLE(3, 2, 3, 2));

    FMatrix res;
    // x.x, y.x, z.x, w.x
    res.row[0] = _mm_shuffle_ps(temp.row[0], temp.row[1], _MM_SHUFFLE(2, 0, 2, 0));
    // x.y, y.y, z.y, w.y
    res.row[1] = _mm_shuffle_ps(temp.row[0], temp.row[1], _MM_SHUFFLE(3, 1, 3, 1));
    // x.z, y.z, z.z, w.z
    res.row[2] = _mm_shuffle_ps(temp.row[2], temp.row[3], _MM_SHUFFLE(2, 0, 2, 0));
    // x.w, y.w, z.w, w.w
    res.row[3] = _mm_shuffle_ps(temp.row[2], temp.row[3], _MM_SHUFFLE(3, 1, 3, 1));
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
float FMatrix::Determinant(const FMatrix& mat) {
#if defined(_XM_SSE_INTRINSICS_)
    static const __m128 Sign = _mm_set_ps(1.f, -1.f, 1.f, -1.f);

    const float* R0 = mat.row[0].m128_f32;
    const float* R1 = mat.row[1].m128_f32;
    const float* R2 = mat.row[2].m128_f32;
    const float* R3 = mat.row[3].m128_f32;

    __m128 V0 = _mm_set_ps(R2[1], R2[0], R2[0], R2[0]);
    __m128 V1 = _mm_set_ps(R3[2], R3[2], R3[1], R3[1]);
    __m128 V2 = _mm_set_ps(R2[1], R2[0], R2[0], R2[0]);
    __m128 V3 = _mm_set_ps(R3[3], R3[3], R3[3], R3[2]);
    __m128 V4 = _mm_set_ps(R2[2], R2[2], R2[1], R2[1]);
    __m128 V5 = _mm_set_ps(R3[3], R3[3], R3[3], R3[2]);

    __m128 P0 = _mm_mul_ps(V0, V1);
    __m128 P1 = _mm_mul_ps(V2, V3);
    __m128 P2 = _mm_mul_ps(V4, V5);


    V0 = _mm_set_ps(R2[2], R2[2], R2[1], R2[1]);
    V1 = _mm_set_ps(R3[1], R3[0], R3[0], R3[0]);
    V2 = _mm_set_ps(R2[3], R2[3], R2[3], R2[2]);
    V3 = _mm_set_ps(R3[1], R3[0], R3[0], R3[0]);
    V4 = _mm_set_ps(R2[3], R2[3], R2[3], R2[2]);
    V5 = _mm_set_ps(R3[2], R3[2], R3[1], R3[1]);

    P0 = _mm_sub_ps(P0, _mm_mul_ps(V0, V1));
    P1 = _mm_sub_ps(P1, _mm_mul_ps(V2, V3));
    P2 = _mm_sub_ps(P2, _mm_mul_ps(V4, V5));


    V0 = _mm_set_ps(R1[3], R1[3], R1[3], R1[2]);
    V1 = _mm_set_ps(R1[2], R1[2], R1[1], R1[1]);
    V2 = _mm_set_ps(R1[1], R1[0], R1[0], R1[0]);

    __m128 S = _mm_mul_ps(mat.row[0], Sign);
    __m128 R = _mm_mul_ps(V0, P0);

    R = _mm_sub_ps(R, _mm_mul_ps(V1, P1));
    R = _mm_add_ps(_mm_mul_ps(V2, P2), R);

    __m128 vTemp2 = R;
    __m128 vTemp = _mm_mul_ps(S, vTemp2);
    vTemp2 = _mm_shuffle_ps(vTemp2, vTemp, _MM_SHUFFLE(1, 0, 0, 0)); // Copy X to the Z position and Y to the W position
    vTemp2 = _mm_add_ps(vTemp2, vTemp);          // Add Z = X+Z; W = Y+W;
    vTemp = _mm_shuffle_ps(vTemp, vTemp2, _MM_SHUFFLE(0, 3, 0, 0));  // Copy W to the Z position
    vTemp = _mm_add_ps(vTemp, vTemp2);           // Add Z and W together
    //return XM_PERMUTE_PS(vTemp, _MM_SHUFFLE(2, 2, 2, 2));
    return vTemp.m128_f32[2];
#else
    // 행렬식 계산 (라플라스 전개, 4x4 행렬)
    float det = 0.0f;
    for (int32 i = 0; i < 4; i++) {
        float subMat[3][3];
        for (int32 j = 1; j < 4; j++) {
            int32 colIndex = 0;
            for (int32 k = 0; k < 4; k++) {
                if (k == i) continue;
                subMat[j - 1][colIndex] = mat.M[j][k];
                colIndex++;
            }
        }
        float minorDet =
            subMat[0][0] * (subMat[1][1] * subMat[2][2] - subMat[1][2] * subMat[2][1]) -
            subMat[0][1] * (subMat[1][0] * subMat[2][2] - subMat[1][2] * subMat[2][0]) +
            subMat[0][2] * (subMat[1][0] * subMat[2][1] - subMat[1][1] * subMat[2][0]);
        det += (i % 2 == 0 ? 1 : -1) * mat.M[0][i] * minorDet;
    }
    return det;
#endif
}

// 역행렬.
FMatrix FMatrix::Inverse(const FMatrix& Mat) {
#if defined(_XM_SSE_INTRINSICS_)
    // DirectXMathMatrix 참고.

    float det = Determinant(Mat);
    if ( fabs(det) < 1e-6 ) {
        return Identity;
    }

    // Transpose matrix
    FMatrix MT = Transpose(Mat);

    __m128 V00 = XM_PERMUTE_PS(MT.row[2], _MM_SHUFFLE(1, 1, 0, 0));
    __m128 V10 = XM_PERMUTE_PS(MT.row[3], _MM_SHUFFLE(3, 2, 3, 2));
    __m128 V01 = XM_PERMUTE_PS(MT.row[0], _MM_SHUFFLE(1, 1, 0, 0));
    __m128 V11 = XM_PERMUTE_PS(MT.row[1], _MM_SHUFFLE(3, 2, 3, 2));
    __m128 V02 = _mm_shuffle_ps(MT.row[2], MT.row[0], _MM_SHUFFLE(2, 0, 2, 0));
    __m128 V12 = _mm_shuffle_ps(MT.row[3], MT.row[1], _MM_SHUFFLE(3, 1, 3, 1));

    __m128 D0 = _mm_mul_ps(V00, V10);
    __m128 D1 = _mm_mul_ps(V01, V11);
    __m128 D2 = _mm_mul_ps(V02, V12);

    V00 = XM_PERMUTE_PS(MT.row[2], _MM_SHUFFLE(3, 2, 3, 2));
    V10 = XM_PERMUTE_PS(MT.row[3], _MM_SHUFFLE(1, 1, 0, 0));
    V01 = XM_PERMUTE_PS(MT.row[0], _MM_SHUFFLE(3, 2, 3, 2));
    V11 = XM_PERMUTE_PS(MT.row[1], _MM_SHUFFLE(1, 1, 0, 0));
    V02 = _mm_shuffle_ps(MT.row[2], MT.row[0], _MM_SHUFFLE(3, 1, 3, 1));
    V12 = _mm_shuffle_ps(MT.row[3], MT.row[1], _MM_SHUFFLE(2, 0, 2, 0));

    D0 = XM_FNMADD_PS(V00, V10, D0);
    D1 = XM_FNMADD_PS(V01, V11, D1);
    D2 = XM_FNMADD_PS(V02, V12, D2);

    // V11 = D0Y,D0W,D2Y,D2Y
    V11 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(1, 1, 3, 1));
    V00 = XM_PERMUTE_PS(MT.row[1], _MM_SHUFFLE(1, 0, 2, 1));
    V10 = _mm_shuffle_ps(V11, D0, _MM_SHUFFLE(0, 3, 0, 2));
    V01 = XM_PERMUTE_PS(MT.row[0], _MM_SHUFFLE(0, 1, 0, 2));
    V11 = _mm_shuffle_ps(V11, D0, _MM_SHUFFLE(2, 1, 2, 1));

    // V13 = D1Y,D1W,D2W,D2W
    __m128 V13 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(3, 3, 3, 1));
    V02 = XM_PERMUTE_PS(MT.row[3], _MM_SHUFFLE(1, 0, 2, 1));
    V12 = _mm_shuffle_ps(V13, D1, _MM_SHUFFLE(0, 3, 0, 2));
    __m128 V03 = XM_PERMUTE_PS(MT.row[2], _MM_SHUFFLE(0, 1, 0, 2));
    V13 = _mm_shuffle_ps(V13, D1, _MM_SHUFFLE(2, 1, 2, 1));

    __m128 C0 = _mm_mul_ps(V00, V10);
    __m128 C2 = _mm_mul_ps(V01, V11);
    __m128 C4 = _mm_mul_ps(V02, V12);
    __m128 C6 = _mm_mul_ps(V03, V13);

    // V11 = D0X,D0Y,D2X,D2X
    V11 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(0, 0, 1, 0));
    V00 = XM_PERMUTE_PS(MT.row[1], _MM_SHUFFLE(2, 1, 3, 2));
    V10 = _mm_shuffle_ps(D0, V11, _MM_SHUFFLE(2, 1, 0, 3));
    V01 = XM_PERMUTE_PS(MT.row[0], _MM_SHUFFLE(1, 3, 2, 3));
    V11 = _mm_shuffle_ps(D0, V11, _MM_SHUFFLE(0, 2, 1, 2));
    // V13 = D1X,D1Y,D2Z,D2Z
    V13 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(2, 2, 1, 0));
    V02 = XM_PERMUTE_PS(MT.row[3], _MM_SHUFFLE(2, 1, 3, 2));
    V12 = _mm_shuffle_ps(D1, V13, _MM_SHUFFLE(2, 1, 0, 3));
    V03 = XM_PERMUTE_PS(MT.row[2], _MM_SHUFFLE(1, 3, 2, 3));
    V13 = _mm_shuffle_ps(D1, V13, _MM_SHUFFLE(0, 2, 1, 2));

    C0 = XM_FNMADD_PS(V00, V10, C0);
    C2 = XM_FNMADD_PS(V01, V11, C2);
    C4 = XM_FNMADD_PS(V02, V12, C4);
    C6 = XM_FNMADD_PS(V03, V13, C6);

    V00 = XM_PERMUTE_PS(MT.row[1], _MM_SHUFFLE(0, 3, 0, 3));
    // V10 = D0Z,D0Z,D2X,D2Y
    V10 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(1, 0, 2, 2));
    V10 = XM_PERMUTE_PS(V10, _MM_SHUFFLE(0, 2, 3, 0));
    V01 = XM_PERMUTE_PS(MT.row[0], _MM_SHUFFLE(2, 0, 3, 1));
    // V11 = D0X,D0W,D2X,D2Y
    V11 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(1, 0, 3, 0));
    V11 = XM_PERMUTE_PS(V11, _MM_SHUFFLE(2, 1, 0, 3));
    V02 = XM_PERMUTE_PS(MT.row[3], _MM_SHUFFLE(0, 3, 0, 3));
    // V12 = D1Z,D1Z,D2Z,D2W
    V12 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(3, 2, 2, 2));
    V12 = XM_PERMUTE_PS(V12, _MM_SHUFFLE(0, 2, 3, 0));
    V03 = XM_PERMUTE_PS(MT.row[2], _MM_SHUFFLE(2, 0, 3, 1));
    // V13 = D1X,D1W,D2Z,D2W
    V13 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(3, 2, 3, 0));
    V13 = XM_PERMUTE_PS(V13, _MM_SHUFFLE(2, 1, 0, 3));

    V00 = _mm_mul_ps(V00, V10);
    V01 = _mm_mul_ps(V01, V11);
    V02 = _mm_mul_ps(V02, V12);
    V03 = _mm_mul_ps(V03, V13);
    __m128 C1 = _mm_sub_ps(C0, V00);
    C0 = _mm_add_ps(C0, V00);
    __m128 C3 = _mm_add_ps(C2, V01);
    C2 = _mm_sub_ps(C2, V01);
    __m128 C5 = _mm_sub_ps(C4, V02);
    C4 = _mm_add_ps(C4, V02);
    __m128 C7 = _mm_add_ps(C6, V03);
    C6 = _mm_sub_ps(C6, V03);

    C0 = _mm_shuffle_ps(C0, C1, _MM_SHUFFLE(3, 1, 2, 0));
    C2 = _mm_shuffle_ps(C2, C3, _MM_SHUFFLE(3, 1, 2, 0));
    C4 = _mm_shuffle_ps(C4, C5, _MM_SHUFFLE(3, 1, 2, 0));
    C6 = _mm_shuffle_ps(C6, C7, _MM_SHUFFLE(3, 1, 2, 0));
    C0 = XM_PERMUTE_PS(C0, _MM_SHUFFLE(3, 1, 2, 0));
    C2 = XM_PERMUTE_PS(C2, _MM_SHUFFLE(3, 1, 2, 0));
    C4 = XM_PERMUTE_PS(C4, _MM_SHUFFLE(3, 1, 2, 0));
    C6 = XM_PERMUTE_PS(C6, _MM_SHUFFLE(3, 1, 2, 0));

    // dot product to calculate determinant
    __m128 vTemp2 = C0;
    __m128 vTemp = _mm_mul_ps(MT.row[0], vTemp2);
    vTemp2 = _mm_shuffle_ps(vTemp2, vTemp, _MM_SHUFFLE(1, 0, 0, 0));
    vTemp2 = _mm_add_ps(vTemp2, vTemp);
    vTemp = _mm_shuffle_ps(vTemp, vTemp2, _MM_SHUFFLE(0, 3, 0, 0));
    vTemp = _mm_add_ps(vTemp, vTemp2);
    vTemp = XM_PERMUTE_PS(vTemp, _MM_SHUFFLE(2, 2, 2, 2));

    vTemp = _mm_div_ps(_mm_set_ps1(1.f), vTemp);
    FMatrix mResult;
    mResult.row[0] = _mm_mul_ps(C0, vTemp);
    mResult.row[1] = _mm_mul_ps(C2, vTemp);
    mResult.row[2] = _mm_mul_ps(C4, vTemp);
    mResult.row[3] = _mm_mul_ps(C6, vTemp);
    return mResult;
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

    FMatrix rotationZ = FMatrix(
        cosYaw, sinYaw, 0.f, 0.f,
        -sinYaw, cosYaw, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f
    );

    // Y축 (Pitch) 회전
    FMatrix rotationY = FMatrix(
        cosPitch, 0.f, -sinPitch, 0.f,
        0.f, 1.f, 0.f, 0.f,
        sinPitch, 0.f, cosPitch, 0.f,
        0.f, 0.f, 0.f, 1.f
    );

    // X축 (Roll) 회전
    FMatrix rotationX = FMatrix(
        1.f, 0.f, 0.f, 0.f,
        0.f, cosRoll, sinRoll, 0.f,
        0.f, -sinRoll, cosRoll, 0.f,
        0.f, 0.f, 0.f, 1.f
    );

    // DirectX 표준 순서: Z(Yaw) → Y(Pitch) → X(Roll)  
    return rotationX * rotationY * rotationZ;  // 이렇게 하면  오른쪽 부터 적용됨
}


// 스케일 행렬 생성
FMatrix FMatrix::CreateScale(float scaleX, float scaleY, float scaleZ)
{
    return FMatrix(
        scaleX, 0.f, 0.f, 0.f,
        0.f, scaleY, 0.f, 0.f,
        0.f, 0.f, scaleZ, 0.f,
        0.f, 0.f, 0.f, 1.f
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

// 행렬을 사용하여 벡터 변환 (W = 1으로 가정)
FVector FMatrix::TransformPosition(const FVector& vector) const {
#if defined(__AVX2__)
    FMatrix MT = FMatrix::Transpose(*this);
    __m256 vec = _mm256_set_ps(
        1.f, vector.z, vector.y, vector.x,
        1.f, vector.z, vector.y, vector.x
    );

    float* xy = _mm256_mul_ps(vec, _mm256_loadu_ps(MT.M[0])).m256_f32;
    float* zw = _mm256_mul_ps(vec, _mm256_loadu_ps(MT.M[2])).m256_f32;
    float w = zw[4] + zw[5] + zw[6] + zw[7];
    if ( w != 0.f ) {
        return FVector(
            (xy[0] + xy[1] + xy[2] + xy[3]) / w,
            (xy[4] + xy[5] + xy[6] + xy[7]) / w,
            (zw[0] + zw[1] + zw[2] + zw[3]) / w
        );
    } else {
        return FVector(
            xy[0] + xy[1] + xy[2] + xy[3],
            xy[4] + xy[5] + xy[6] + xy[7],
            zw[0] + zw[1] + zw[2] + zw[3]
        );
    }

#elif defined(_XM_SSE_INTRINSICS_)
    FMatrix MT = FMatrix::Transpose(*this);

    __m128 vec = _mm_set_ps(1.f, vector.z, vector.y, vector.x);
    float* xx = _mm_mul_ps(vec, MT.row[0]).m128_f32;
    float* yy = _mm_mul_ps(vec, MT.row[1]).m128_f32;
    float* zz = _mm_mul_ps(vec, MT.row[2]).m128_f32;
    float* ww = _mm_mul_ps(vec, MT.row[3]).m128_f32;
    float w = ww[0] + ww[1] + ww[2] + ww[3];
    if (w != 0.f) {
        return FVector(
            (xx[0] + xx[1] + xx[2] + xx[3]) / w,
            (yy[0] + yy[1] + yy[2] + yy[3]) / w,
            (zz[0] + zz[1] + zz[2] + zz[3]) / w
        );
    } else {
        return FVector(
            xx[0] + xx[1] + xx[2] + xx[3],
            yy[0] + yy[1] + yy[2] + yy[3],
            zz[0] + zz[1] + zz[2] + zz[3]
        );
    }
#else
    float x = M[0][0] * vector.x + M[1][0] * vector.y + M[2][0] * vector.z + M[3][0];
    float y = M[0][1] * vector.x + M[1][1] * vector.y + M[2][1] * vector.z + M[3][1];
    float z = M[0][2] * vector.x + M[1][2] * vector.y + M[2][2] * vector.z + M[3][2];
    float w = M[0][3] * vector.x + M[1][3] * vector.y + M[2][3] * vector.z + M[3][3];
    return w != 0.0f ? FVector{ x / w, y / w, z / w } : FVector{ x, y, z };
#endif
}

// 4x4 행렬을 사용하여 벡터 변환 (W = 0으로 가정, 방향 벡터)
FVector FMatrix::TransformVector(const FVector& v, const FMatrix& m)
{
#if defined(__AVX2__)
    FMatrix MT = FMatrix::Transpose(m);
    // 엔디안 때문에 거꾸로 넣어줘야 함.
    __m256 vec = _mm256_set_ps(
        0.f, v.z, v.y, v.x,
        0.f, v.z, v.y, v.x
    );
    float* xy = _mm256_mul_ps(vec, _mm256_loadu_ps(MT.M[0])).m256_f32;
    float* zw = _mm256_mul_ps(vec, _mm256_loadu_ps(MT.M[2])).m256_f32;
    return FVector(
        xy[0] + xy[1] + xy[2],
        xy[4] + xy[5] + xy[6],
        zw[0] + zw[1] + zw[2]
    );
#elif defined(_XM_SSE_INTRINSICS_)
    FMatrix MT = FMatrix::Transpose(m);

    __m128 vec = _mm_set_ps(0.f, v.z, v.y, v.x);
    float* xx = _mm_mul_ps(vec, MT.row[0]).m128_f32;
    float* yy = _mm_mul_ps(vec, MT.row[1]).m128_f32;
    float* zz = _mm_mul_ps(vec, MT.row[2]).m128_f32;
    return FVector(
        xx[0] + xx[1] + xx[2],
        yy[0] + yy[1] + yy[2],
        zz[0] + zz[1] + zz[2]
    );
#else
    FVector result;
    
    result.x = v.x * m.M[0][0] + v.y * m.M[1][0] + v.z * m.M[2][0] + 0.0f * m.M[3][0];
    result.y = v.x * m.M[0][1] + v.y * m.M[1][1] + v.z * m.M[2][1] + 0.0f * m.M[3][1];
    result.z = v.x * m.M[0][2] + v.y * m.M[1][2] + v.z * m.M[2][2] + 0.0f * m.M[3][2];

    return result;
#endif
}

// FVector4를 변환하는 함수
FVector4 FMatrix::TransformVector(const FVector4& v, const FMatrix& m)
{
#if defined(__AVX2__)
    FMatrix MT = FMatrix::Transpose(m);
    __m256 vec = _mm256_set_ps(
        0.f, v.z, v.y, v.x,
        0.f, v.z, v.y, v.x
    );
    float* xy = _mm256_mul_ps(vec, _mm256_loadu_ps(MT.M[0])).m256_f32;
    float* zw = _mm256_mul_ps(vec, _mm256_loadu_ps(MT.M[2])).m256_f32;
    return FVector4(
        xy[0] + xy[1] + xy[2] + xy[3],
        xy[4] + xy[5] + xy[6] + xy[7],
        zw[0] + zw[1] + zw[2] + zw[3],
        zw[4] + zw[5] + zw[6] + zw[7]
    );
#elif defined(_XM_SSE_INTRINSICS_)
    FMatrix MT = FMatrix::Transpose(m);

    __m128 vec = _mm_set_ps(0.f, v.z, v.y, v.x);
    float* xx = _mm_mul_ps(vec, MT.row[0]).m128_f32;
    float* yy = _mm_mul_ps(vec, MT.row[1]).m128_f32;
    float* zz = _mm_mul_ps(vec, MT.row[2]).m128_f32;
    float* ww = _mm_mul_ps(vec, MT.row[3]).m128_f32;
    return FVector4(
        xx[0] + xx[1] + xx[2] + xx[3],
        yy[0] + yy[1] + yy[2] + yy[3],
        zz[0] + zz[1] + zz[2] + zz[3],
        ww[0] + ww[1] + ww[2] + ww[3]
    );
#else
    FVector4 result;
    result.x = v.x * m.M[0][0] + v.y * m.M[1][0] + v.z * m.M[2][0] + v.a * m.M[3][0];
    result.y = v.x * m.M[0][1] + v.y * m.M[1][1] + v.z * m.M[2][1] + v.a * m.M[3][1];
    result.z = v.x * m.M[0][2] + v.y * m.M[1][2] + v.z * m.M[2][2] + v.a * m.M[3][2];
    result.a = v.x * m.M[0][3] + v.y * m.M[1][3] + v.z * m.M[2][3] + v.a * m.M[3][3];
    return result;
#endif
}

FBoundingBox::FBoundingBox(FVector _min, FVector _max): min(_min), max(_max)
{}

void FBoundingBox::ExpandToInclude(const FBoundingBox& Other)
{
    min.x = FMath::Min(min.x, Other.min.x);
    min.y = FMath::Min(min.y, Other.min.y);
    min.z = FMath::Min(min.z, Other.min.z);
    
    max.x = FMath::Max(max.x, Other.max.x);
    max.y = FMath::Max(max.y, Other.max.y);
    max.z = FMath::Max(max.z, Other.max.z);
}

FBoundingBox FBoundingBox::ComputeSceneBoundingBox(const TSet<class AActor*>& SpawnedActors)
{
    if (SpawnedActors.IsEmpty())
    {
        return FBoundingBox();
    }

    FBoundingBox result;
    for (auto SpawnedActor : SpawnedActors)
    {
        if (SpawnedActor->IsA(UTransformGizmo::StaticClass())) continue;
        
        UPrimitiveComponent* spawnedPrim = SpawnedActor->GetComponentByClass<UPrimitiveComponent>();
        if (spawnedPrim == nullptr) continue;

        FMatrix Model = JungleMath::CreateModelMatrix(
            spawnedPrim->GetWorldLocation(),
            spawnedPrim->GetWorldRotation(),
            spawnedPrim->GetWorldScale()
        );

        FBoundingBox WorldBoundingBox = FBoundingBox::TransformBy(spawnedPrim->AABB, spawnedPrim->GetWorldLocation(), Model);
        result.ExpandToInclude(WorldBoundingBox);
    }

    return result;
}

FBoundingBox FBoundingBox::TransformBy(const FBoundingBox& localAABB, const FVector& center, const FMatrix& modelMatrix)
{
    FVector localVertices[8] = {
        { localAABB.min.x, localAABB.min.y, localAABB.min.z },
        { localAABB.max.x, localAABB.min.y, localAABB.min.z },
        { localAABB.min.x, localAABB.max.y, localAABB.min.z },
        { localAABB.max.x, localAABB.max.y, localAABB.min.z },
        { localAABB.min.x, localAABB.min.y, localAABB.max.z },
        { localAABB.max.x, localAABB.min.y, localAABB.max.z },
        { localAABB.min.x, localAABB.max.y, localAABB.max.z },
        { localAABB.max.x, localAABB.max.y, localAABB.max.z }
    };

    FVector worldVertices[8];
    worldVertices[0] = center + FMatrix::TransformVector(localVertices[0], modelMatrix);

    FVector min = worldVertices[0], max = worldVertices[0];

    // 첫 번째 값을 제외한 나머지 버텍스를 변환하고 min/max 계산
    for (int i = 1; i < 8; ++i)
    {
        worldVertices[i] = center + FMatrix::TransformVector(localVertices[i], modelMatrix);

        min.x = (worldVertices[i].x < min.x) ? worldVertices[i].x : min.x;
        min.y = (worldVertices[i].y < min.y) ? worldVertices[i].y : min.y;
        min.z = (worldVertices[i].z < min.z) ? worldVertices[i].z : min.z;

        max.x = (worldVertices[i].x > max.x) ? worldVertices[i].x : max.x;
        max.y = (worldVertices[i].y > max.y) ? worldVertices[i].y : max.y;
        max.z = (worldVertices[i].z > max.z) ? worldVertices[i].z : max.z;
    }
    FBoundingBox BoundingBox;
    BoundingBox.min = min;
    BoundingBox.max = max;

    return BoundingBox;
}
