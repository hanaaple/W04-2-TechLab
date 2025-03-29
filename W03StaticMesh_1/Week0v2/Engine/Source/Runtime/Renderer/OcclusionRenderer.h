#pragma once
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
    void Prepare();
    void IssueQueries(TArray<UStaticMeshComponent*>& StaticMeshObjs, const FMatrix& VP);
    void IssueQuery(const FBoundingBox& box, const FMatrix& MVP, const FOcclusionQuery& query);
    void ResolveQueries(const TArray<UStaticMeshComponent*>& StaticMeshObjs);
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
        FMatrix MVP;
    };

private:
    FGraphicsDevice* Graphics;
};

