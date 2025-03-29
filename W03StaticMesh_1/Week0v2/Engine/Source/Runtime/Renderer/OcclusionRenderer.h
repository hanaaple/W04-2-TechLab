#pragma once
#include "Math/Vector.h"
#include "Math/Matrix.h"
class FGraphicsDevice;
class ID3D11VertexShader;
class ID3D11PixelShader;
class ID3D11InputLayout;
class ID3D11Buffer;
class FBoundingBox;
class FOcclusionRenderer
{
public:
    void Initialize(FGraphicsDevice* graphics);
    void Prepare();
    void IssueQueries(const FBoundingBox& box, const FMatrix& MVP);
    void ResolveQueries();
    void Release();

private:
    void CreateShader();
    void CreateConstantBuffer();
    void UpdateMVPConstantBuffer(const FMatrix& mvp);
    ID3D11VertexShader* OcclusionVertexShader = nullptr;
    ID3D11InputLayout* InputLayout = nullptr;
    ID3D11Buffer* MVPConstantBuffer = nullptr;

    struct MVPConstant {
        FMatrix MVP;
    };

private:
    FGraphicsDevice* Graphics;
};

