#pragma once

#include <DirectXMath.h>

struct FVector2D
{
	float x,y;
	FVector2D(float _x = 0, float _y = 0) : x(_x), y(_y) {}

	FVector2D operator+(const FVector2D& rhs) const
	{
		return FVector2D(x + rhs.x, y + rhs.y);
	}
	FVector2D operator-(const FVector2D& rhs) const
	{
		return FVector2D(x - rhs.x, y - rhs.y);
	}
	FVector2D operator*(float rhs) const
	{
		return FVector2D(x * rhs, y * rhs);
	}
	FVector2D operator/(float rhs) const
	{
		return FVector2D(x / rhs, y / rhs);
	}
	FVector2D& operator+=(const FVector2D& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}
};

// 3D 벡터
struct FVector
{

    float x, y, z;
    FVector(float _x = 0, float _y = 0, float _z = 0): x(_x), y(_y), z(_z) {}

    FVector operator+(const FVector& other) const;
    FVector operator-(const FVector& other) const;
    

    // 벡터 내적
    float Dot(const FVector& other) const;

    float MagnitudePow() const;

    // 벡터 크기
    float Magnitude() const;

    // 벡터 정규화
    FVector Normalize() const;
    FVector Cross(const FVector& Other) const;

    // 스칼라 곱셈
    FVector operator*(float scalar) const;
    bool operator==(const FVector& other) const {
        return (x == other.x && y == other.y && z == other.z);
    }

    float Distance(const FVector& other) const {
        // 두 벡터의 차 벡터의 크기를 계산
        return ((*this - other).Magnitude());
    }
    DirectX::XMFLOAT3 ToXMFLOAT3() const
    {
        return DirectX::XMFLOAT3(x, y, z);
    }

    static const FVector ZeroVector;
    static const FVector OneVector;
    static const FVector UpVector;
    static const FVector ForwardVector;
    static const FVector RightVector;
};
