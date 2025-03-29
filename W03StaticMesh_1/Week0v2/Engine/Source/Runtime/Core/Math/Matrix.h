#pragma once

#include <DirectXMath.h>

// 4x4 행렬 연산
union FMatrix
{
	float M[4][4];
    __m128 row[4];
    __m256 _rowin256[2];
	static const FMatrix Identity;

    FMatrix() noexcept;

    constexpr FMatrix(
        float m00, float m10, float m20, float m30,
        float m01, float m11, float m21, float m31,
        float m02, float m12, float m22, float m32,
        float m03, float m13, float m23, float m33
    ) noexcept;
	// 기본 연산자 오버로딩
	FMatrix operator+(const FMatrix& Other) const;
	FMatrix operator-(const FMatrix& Other) const;
	FMatrix operator*(const FMatrix& Other) const;
	FMatrix operator*(float Scalar) const;
	FMatrix operator/(float Scalar) const;
	float* operator[](int row);
	const float* operator[](int row) const;
	
	// 유틸리티 함수
	static FMatrix Transpose(const FMatrix& Mat);
	static float Determinant(const FMatrix& Mat);
	static FMatrix Inverse(const FMatrix& Mat);
	static FMatrix CreateRotation(float roll, float pitch, float yaw);
	static FMatrix CreateScale(float scaleX, float scaleY, float scaleZ);
	static FVector TransformVector(const FVector& v, const FMatrix& m);
	static FVector4 TransformVector(const FVector4& v, const FMatrix& m);
	static FMatrix CreateTranslationMatrix(const FVector& position);
	static FMatrix CreateInverseMatrixWithSRT(
        const FVector& scale, 
        const FVector& rotation, 
        const FVector& position
    );


	DirectX::XMMATRIX ToXMMATRIX() const
	{
		return DirectX::XMMatrixSet(
			M[0][0], M[1][0], M[2][0], M[3][0], // 첫 번째 열
			M[0][1], M[1][1], M[2][1], M[3][1], // 두 번째 열
			M[0][2], M[1][2], M[2][2], M[3][2], // 세 번째 열
			M[0][3], M[1][3], M[2][3], M[3][3]  // 네 번째 열
		);
	}
	FVector TransformPosition(const FVector& vector) const;
};