#include "Vector.h"

const FVector FVector::ZeroVector = {0.0f, 0.0f, 0.0f};
const FVector FVector::OneVector = {1.0f, 1.0f, 1.0f};
const FVector FVector::ForwardVector = {1.0f, 0.0f, 0.0f};
const FVector FVector::RightVector = {0.0f, 1.0f, 0.0f};
const FVector FVector::UpVector = {0.0f, 0.0f, 1.0f};

FVector FVector::operator+(const FVector& other) const {
#if defined(_XM_SSE_INTRINSICS_)
    __m128 vTemp1 = _mm_set_ps(0.f, z, y, x);
    __m128 vTemp2 = _mm_set_ps(0.f, other.z, other.y, other.x);
    float temp[4];
    _mm_store_ps(temp, _mm_add_ps(vTemp1, vTemp2));
    return FVector(temp[0], temp[1], temp[2]);
#else
    return FVector(x + other.x, y + other.y, z + other.z);
#endif
}

FVector FVector::operator-(const FVector& other) const {
#if defined(_XM_SSE_INTRINSICS_)
    __m128 vTemp1 = _mm_set_ps(0.f, z, y, x);
    __m128 vTemp2 = _mm_set_ps(0.f, other.z, other.y, other.x);
    float temp[4];
    _mm_store_ps(temp, _mm_sub_ps(vTemp1, vTemp2));
    return FVector(temp[0], temp[1], temp[2]);
#else
    return FVector(x - other.x, y - other.y, z - other.z);
#endif
}

FVector FVector::operator*(float scalar) const {
#if defined(_XM_SSE_INTRINSICS_)
    __m128 vTemp1 = _mm_set_ps(0.f, z, y, x);
    __m128 vTemp2 = _mm_set_ps1(scalar);
    float temp[4];
    _mm_store_ps(temp, _mm_mul_ps(vTemp1, vTemp2));
    return FVector(temp[0], temp[1], temp[2]);
#else
    return FVector(x * scalar, y * scalar, z * scalar);
#endif
}

// 벡터 내적
float FVector::Dot(const FVector& other) const {
#if defined(_XM_SSE_INTRINSICS_)
    __m128 vTemp2 = _mm_set_ps(0.f, other.z, other.y, other.x);
    __m128 vTemp1 = _mm_set_ps(0.f, z, y, x);
    __m128 vTemp = _mm_mul_ps(vTemp1, vTemp2);
    vTemp = _mm_hadd_ps(vTemp, vTemp);
    vTemp = _mm_hadd_ps(vTemp, vTemp);
    return vTemp.m128_f32[0];
#else 
    return x * other.x + y * other.y + z * other.z;
#endif
}

// 벡터 크기
float FVector::MagnitudePow() const {
#if defined(_XM_SSE_INTRINSICS_)
    __m128 vTemp2 = _mm_set_ps(0.f, z, y, x);
    __m128 vTemp1 = _mm_set_ps(0.f, z, y, x);
    __m128 vTemp = _mm_mul_ps(vTemp1, vTemp2);
    vTemp = _mm_hadd_ps(vTemp, vTemp);
    vTemp = _mm_hadd_ps(vTemp, vTemp);
    return vTemp.m128_f32[0];
#else 
    return x * other.x + y * other.y + z * other.z;
#endif
}

float FVector::Magnitude() const {
    return sqrt(MagnitudePow());
}

// 벡터 정규화

FVector FVector::Normalize() const {
    float mag = Magnitude();
    if ( mag == 0 )
        return FVector(0, 0, 0);
#if defined(_XM_SSE_INTRINSICS_)
    float vTemp[4];
    __m128 RS = _mm_set_ps1(1 / mag);
    _mm_store_ps(vTemp, _mm_mul_ps(_mm_set_ps(0.f, z, y, x), RS));
    return FVector(vTemp[0], vTemp[1], vTemp[2]);
#else
    return FVector(x / mag, y / mag, z / mag);
#endif
}

FVector FVector::Cross(const FVector& Other) const {
#if defined(_XM_SSE_INTRINSICS_)
    // reference : https://geometrian.com/resources/cross_product/
    __m128 vec0 = _mm_set_ps(0.f, z, y, x);
    __m128 vec1 = _mm_set_ps(0.f, Other.z, Other.y, Other.x);
    float vTemp[4];

    // for intel
    __m128 tmp0 = _mm_shuffle_ps(vec0, vec0, _MM_SHUFFLE(3, 0, 2, 1));
    __m128 tmp1 = _mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 1, 0, 2));
    __m128 tmp2 = _mm_mul_ps(tmp0, vec1);
    __m128 tmp3 = _mm_mul_ps(tmp0, tmp1);
    __m128 tmp4 = _mm_shuffle_ps(tmp2, tmp2, _MM_SHUFFLE(3, 0, 2, 1));
    _mm_store_ps(vTemp, _mm_sub_ps(tmp3, tmp4));
    // for amd
    //__m128 tmp0 = _mm_shuffle_ps(vec0, vec0, _MM_SHUFFLE(3, 0, 2, 1));
    //__m128 tmp1 = _mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 1, 0, 2));
    //__m128 tmp2 = _mm_shuffle_ps(vec0, vec0, _MM_SHUFFLE(3, 1, 0, 2));
    //__m128 tmp3 = _mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(3, 0, 2, 1));
    //__m128 tmp4 = _mm_sub_ps(_mm_mul_ps(tmp0, tmp1), _mm_mul_ps(tmp2, tmp3));
    //_mm_store_ps(vTemp, tmp4);
    return FVector(vTemp[0], vTemp[1], vTemp[2]);
#else
    return FVector{
        y * Other.z - z * Other.y,
        z * Other.x - x * Other.z,
        x * Other.y - y * Other.x
    };
#endif
}
