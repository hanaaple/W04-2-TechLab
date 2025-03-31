#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#define _TCHAR_DEFINED
#include <d3d11.h>
#include "EngineBaseTypes.h"
#include "Define.h"
#include "Container/Map.h"
#include "Container/Set.h"

class ULightComponentBase;
class UWorld;
class FGraphicsDevice;
class UMaterial;
struct FStaticMaterial;
class UObject;
class FEditorViewportClient;
class UBillboardComponent;
class UStaticMeshComponent;
class UGizmoBaseComponent;
class FOcclusionRenderer;
class FRenderer

#define UseBufferDynamic 0
#define MaxBufferSize 64 * 1024 * 1024
{

private:
    float litFlag = 0;

public:
    FGraphicsDevice* Graphics;
    ID3D11VertexShader* VertexShader = nullptr;
    ID3D11PixelShader* PixelShader = nullptr;
    ID3D11InputLayout* InputLayout = nullptr;

    ID3D11Buffer* ConstantBuffer = nullptr;
    ID3D11Buffer* LightingConstantBuffer = nullptr;
    ID3D11Buffer* FlagConstantBuffer = nullptr;
    ID3D11Buffer* MaterialConstantBuffer = nullptr;
    ID3D11Buffer* SubMeshConstantBuffer = nullptr;
    ID3D11Buffer* TextureConstantBuffer = nullptr;
    ID3D11Buffer* CameraConstantBuffer = nullptr;

    FLighting lightingData;

    uint32 Stride;
    uint32 Stride2;

public:
    void Initialize(FGraphicsDevice* graphics);
    
    void PrepareShader();
    
    //Render
    //void RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices) const;
    //void RenderPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices) const;
    void RenderPrimitive(OBJ::FStaticMeshRenderData* renderData, TArray<FStaticMaterial*> materials, TArray<UMaterial*> overrideMaterial, int selectedSubMeshIndex);
   
    void RenderTexturedModelPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices, ID3D11ShaderResourceView* InTextureSRV, ID3D11SamplerState* InSamplerState);
    //Release
    void Release();
    void ReleaseShader();
    void ReleaseBuffer(ID3D11Buffer*& Buffer) const;
    void ReleaseConstantBuffer();

    void ResetVertexShader();
    void ResetPixelShader();
    void CreateShader();

    void CreateVertexShader(const FWString& filename, const FString& funcname, const FString& version);
    void CreatePixelShader(const FWString& filename, const FString& funcname, const FString& version);
    
    void ChangeViewMode(EViewModeIndex evi);
    
    // CreateBuffer
    void CreateConstantBuffer();
    void CreateLightingConstantBuffer();
    void CreateLitUnlitBuffer();

    ID3D11Buffer* CreateBuffer(void* Data, uint32 ByteWidth, D3D11_USAGE Usage, D3D11_CPU_ACCESS_FLAG Flag, D3D11_BIND_FLAG BindFlag);
    ID3D11Buffer* CreateVertexBuffer(FVertexSimple* vertices, UINT byteWidth) const;
    ID3D11Buffer* CreateVertexBuffer(const TArray<FVertexSimple>& vertices, UINT byteWidth) const;
    ID3D11Buffer* CreateIndexBuffer(uint32* indices, UINT byteWidth) const;
    ID3D11Buffer* CreateIndexBuffer(const TArray<uint32>& indices, UINT byteWidth) const;

    // update
    void UpdateLightConstantBuffer() const;
    //void UpdateConstant(const FMatrix& MVP, const FMatrix& NormalMatrix, FVector4 UUIDColor, bool IsSelected) const;
    void UpdateConstant(const FMatrix& MVP) const;
    void UpdateMaterial(const FObjMaterialInfo& MaterialInfo);
    void UpdateLitUnlitConstant(int32 isLit);
    void UpdateSubMeshConstant(bool isSelected) const;
    void UpdateTextureConstant(float UOffset, float VOffset);
    void UpdateCameraConstant(FEditorViewportClient* ActiveViewport) const;

public://텍스쳐용 기능 추가
    ID3D11VertexShader* VertexTextureShader = nullptr;
    ID3D11PixelShader* PixelTextureShader = nullptr;
    ID3D11InputLayout* TextureInputLayout = nullptr;

    uint32 TextureStride;
    struct FSubUVConstant
    {
        float indexU;
        float indexV;
    };
    ID3D11Buffer* SubUVConstantBuffer = nullptr;

public:
    void CreateTextureShader();
    void ReleaseTextureShader();
    void PrepareTextureShader();
    ID3D11Buffer* CreateVertexTextureBuffer(FVertexTexture* vertices, UINT byteWidth) const;
    ID3D11Buffer* CreateIndexTextureBuffer(uint32* indices, UINT byteWidth) const;
    void RenderTexturePrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices,
        ID3D11Buffer* pIndexBuffer, UINT numIndices,
        ID3D11ShaderResourceView* _TextureSRV,
        ID3D11SamplerState* _SamplerState);
    void RenderTextPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices,
        ID3D11ShaderResourceView* _TextureSRV,
        ID3D11SamplerState* _SamplerState);
    ID3D11Buffer* CreateVertexBuffer(FVertexTexture* vertices, UINT byteWidth) const;

    void UpdateSubUVConstant(float _indexU, float _indexV) const;
    void PrepareSubUVConstant();


public: // line shader
    void PrepareLineShader();
    void CreateLineShader();
    void ReleaseLineShader() const;
    void RenderBatch(const FGridParameters& gridParam, ID3D11Buffer* pVertexBuffer, int boundingBoxCount, int coneCount, int coneSegmentCount, int obbCount);
    void UpdateGridConstantBuffer(const FGridParameters& gridParams) const;
    void UpdateLinePrimitveCountBuffer(int numBoundingBoxes, int numCones) const;
    ID3D11Buffer* CreateStaticVerticesBuffer() const;
    ID3D11Buffer* CreateBoundingBoxBuffer(UINT numBoundingBoxes) const;
    ID3D11Buffer* CreateOBBBuffer(UINT numBoundingBoxes) const;
    ID3D11Buffer* CreateConeBuffer(UINT numCones) const;
    ID3D11ShaderResourceView* CreateBoundingBoxSRV(ID3D11Buffer* pBoundingBoxBuffer, UINT numBoundingBoxes);
    ID3D11ShaderResourceView* CreateOBBSRV(ID3D11Buffer* pBoundingBoxBuffer, UINT numBoundingBoxes);
    ID3D11ShaderResourceView* CreateConeSRV(ID3D11Buffer* pConeBuffer, UINT numCones);

    void CreateBatchRenderCache();
    void UpdateOrCreateBuffer(const FString& MaterialName, uint32 BufferIndex, FVertexSimple* VertexData, uint32 VertexDataSize, void* IndexData, uint32
                              IndexDataSize, uint32 IndexDataCount, uint32 BufferSize);
    void BakeBatchRenderBuffer();
    void RenderBakedBuffer();
    void ReleaseUnUsedBatchBuffer(const FString& MaterialName, uint32 ReleaseStartBufferIndex);

    void UpdateBoundingBoxBuffer(ID3D11Buffer* pBoundingBoxBuffer, const TArray<FBoundingBox>& BoundingBoxes, int numBoundingBoxes) const;
    void UpdateOBBBuffer(ID3D11Buffer* pBoundingBoxBuffer, const TArray<FOBB>& BoundingBoxes, int numBoundingBoxes) const;
    void UpdateConesBuffer(ID3D11Buffer* pConeBuffer, const TArray<FCone>& Cones, int numCones) const;

    //Render Pass Demo
    void PrepareRender();
    void ClearRenderArr();
    void Render(UWorld* World, const std::shared_ptr<FEditorViewportClient>& ActiveViewport);
    void RenderStaticMeshes(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void RenderGizmos(const UWorld* World, const std::shared_ptr<FEditorViewportClient>& ActiveViewport);
    void RenderLight(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void RenderBillboards(UWorld* World,std::shared_ptr<FEditorViewportClient> ActiveViewport);

public:
    void UpdateBatchRenderTarget(std::shared_ptr<FEditorViewportClient> ActiveViewport);
    
public:
    void SetTopology(const D3D11_PRIMITIVE_TOPOLOGY InPrimitiveTopology);
    void SetPSTextureSRV(uint32 StartSlot, uint32 NumViews, ID3D11ShaderResourceView* InSRV);
    void SetPSSamplerState(uint32 StartSlot, uint32 NumSamplers, ID3D11SamplerState* InSamplerState);
    void SetVertexShader(ID3D11VertexShader* InVertexShader);
    void SetPixelShader(ID3D11PixelShader* InPixelShader);
    void SetInputLayout(ID3D11InputLayout* InInputLayout);
    void SetVSConstantBuffers(uint32 StartSlot, uint32 NumBuffers, ID3D11Buffer* InConstantBufferPtr);
    void SetPSConstantBuffers(uint32 StartSlot, uint32 NumBuffers, ID3D11Buffer* InConstantBufferPtr);

    
private:
    TArray<UStaticMeshComponent*> StaticMeshObjs;
    TArray<UGizmoBaseComponent*> GizmoObjs;
    TArray<UBillboardComponent*> BillboardObjs;
    TArray<ULightComponentBase*> LightObjs;

    // VertexBuffer 1개에 IndexBuffer 1 ~ n개 
        
    
    // Topology가 바뀔 일은 없고
    struct BatchRenderTargetContext
    {
        // SubMeshIndex
        TArray<TPair<uint32, UStaticMeshComponent*>> StaticMeshes;

        // UISOO TODO: 버텍스 버퍼, 인덱스 버퍼 나누기도 생각
        bool bIsDirty = true;
    };

    // 머티리얼 이름.
    TMap<FString, BatchRenderTargetContext> BatchRenderTargets;
    

public:
    ID3D11VertexShader* VertexLineShader = nullptr;
    ID3D11PixelShader* PixelLineShader = nullptr;
    ID3D11Buffer* GridConstantBuffer = nullptr;
    ID3D11Buffer* LinePrimitiveBuffer = nullptr;
    ID3D11ShaderResourceView* pBBSRV = nullptr;
    ID3D11ShaderResourceView* pConeSRV = nullptr;
    ID3D11ShaderResourceView* pOBBSRV = nullptr;

private:
    uint8 bFlagIsLit : 1 = 0;

private:
    D3D11_PRIMITIVE_TOPOLOGY CurrentPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    TMap<uint32, TPair<uint32, ID3D11ShaderResourceView*>> CurrentTextureSRV;
    TMap<uint32, TPair<uint32, ID3D11SamplerState*>> CurrentSamplerState;
    ID3D11VertexShader* CurrentVertexShader = nullptr;
    ID3D11PixelShader* CurrentPixelShader = nullptr;
    ID3D11InputLayout* CurrentInputLayout = nullptr;
    TMap<uint32, TPair<uint32, ID3D11Buffer*>> CurrentVSConstantBuffers;
    TMap<uint32, TPair<uint32, ID3D11Buffer*>> CurrentPSConstantBuffers;

    // Material, BufferIndex, VertexBuffer, IndexBufferCount, IndexBuffer
    TMap<FString, TMap<uint32, TPair<ID3D11Buffer*, TPair<uint32, ID3D11Buffer*>>>> CachedBuffers;

    struct VIBuffer {
        TArray<ID3D11Buffer*> VertexBuffer;
        TArray<ID3D11Buffer*> IndexBuffer;
        uint32 Stride;
        TArray<uint32> IndexCount;
    };
    TMap<FString, VIBuffer> BakedBuffers;
    
public:
    void IssueOcclusionQueries(const std::shared_ptr<FEditorViewportClient>& ActiveViewport);
    void ResolveOcclusionQueries();
private:
    FOcclusionRenderer* OcclusionRenderer = nullptr;
    bool bWasOcculusionQueried = false;
};

