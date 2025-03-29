#pragma once
class FRenderer;
class FGraphicsDevice;
class ID3D11VertexShader;
class ID3D11InputLayout;
class ID3D11Buffer;
class FBoundingBox;
class FOcclusionQuery;
class UStaticMeshComponent;

class FOcclusionRenderer
{
public:
    void Initialize(FGraphicsDevice* graphics);
    void Prepare(FRenderer* Renderer);
    void IssueQueries(FRenderer* Renderer);
    void IssueQuery(const FBoundingBox& box, const FMatrix& M, const FOcclusionQuery& query);
    void ResolveQueries();
    void Release();

private:
    void CreateShader();
    void CreateConstantBuffer();
    void CreateVertexBuffer();
    void UpdateBoxConstantBuffer(const FBoundingBox& box, const FMatrix& mvp);
    void Rendering(const FBoundingBox& box);
    ID3D11VertexShader* OcclusionVertexShader = nullptr;
    ID3D11Buffer* BoxConstantBuffer = nullptr;
    ID3D11Buffer* DummyVertexBuffer = nullptr;

    struct alignas(16) FBoxConstantBuffer
    {
        FVector Min;
        float pad0 = 0;
        FVector Max;
        float pad1 = 0;
        FMatrix M;
    };

private:
    FGraphicsDevice* Graphics;
};

