#include "Renderer.h"
#include <d3dcompiler.h>

#include "World.h"
#include "Actors/Player.h"
#include "BaseGizmos/GizmoBaseComponent.h"
#include "Components/LightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/UBillboardComponent.h"
#include "Components/UParticleSubUVComp.h"
#include "Components/UText.h"
#include "Components/Material/Material.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Launch/EngineLoop.h"
#include "Math/JungleMath.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/PrimitiveBatch.h"
#include "UObject/Casts.h"
#include "PropertyEditor/ShowFlags.h"
#include "UObject/UObjectIterator.h"
#include "Components/SkySphereComponent.h"
#include "Engine/FLoaderOBJ.h"

#include "Stats/ScopeCycleCounter.h"
#include "OcclusionRenderer.h"
void FRenderer::Initialize(FGraphicsDevice* graphics)
{
    Graphics = graphics;
    CreateShader();
    CreateTextureShader();
    CreateLineShader();
    CreateConstantBuffer();

    ChangeViewMode(EViewModeIndex::VMI_Lit);    
    OcclusionRenderer = new FOcclusionRenderer();
    OcclusionRenderer->Initialize(graphics);
}

void FRenderer::Release()
{
    ReleaseShader();
    ReleaseTextureShader();
    ReleaseLineShader();
    ReleaseConstantBuffer();
    delete OcclusionRenderer;
}

void FRenderer::CreateShader()
{
    ID3DBlob* VertexShaderCSO;
    ID3DBlob* PixelShaderCSO;
#ifdef _DEBUG
    DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
    shaderFlags |= D3DCOMPILE_DEBUG;
    shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
    // 릴리즈 빌드에서는 최적화가 적용됩니다.
#endif

    D3DCompileFromFile(L"Shaders/StaticMeshVertexShader.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", shaderFlags, 0, &VertexShaderCSO, nullptr);
    Graphics->Device->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &VertexShader);

    D3DCompileFromFile(L"Shaders/StaticMeshPixelShader.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", shaderFlags, 0, &PixelShaderCSO, nullptr);
    Graphics->Device->CreatePixelShader(PixelShaderCSO->GetBufferPointer(), PixelShaderCSO->GetBufferSize(), nullptr, &PixelShader);

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        // {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        // {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        // {"MATERIAL_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    
    Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &InputLayout
    );

    Stride = sizeof(FVertexSimple);
    VertexShaderCSO->Release();
    PixelShaderCSO->Release();
}

void FRenderer::ReleaseShader()
{
    if (InputLayout)
    {
        InputLayout->Release();
        InputLayout = nullptr;
    }

    if (PixelShader)
    {
        PixelShader->Release();
        PixelShader = nullptr;
    }

    if (VertexShader)
    {
        VertexShader->Release();
        VertexShader = nullptr;
    }
}

void FRenderer::PrepareShader()
{
    SetVertexShader(VertexShader);
    SetPixelShader(PixelShader);
    SetInputLayout(InputLayout);

    if (ConstantBuffer)
    {
        SetVSConstantBuffers(0, 1, ConstantBuffer);
        // SetPSConstantBuffers(0, 1, ConstantBuffer);
        SetPSConstantBuffers(1, 1, MaterialConstantBuffer);
        //SetPSConstantBuffers(2, 1, LightingConstantBuffer);
        //SetPSConstantBuffers(3, 1, FlagConstantBuffer);
        //SetPSConstantBuffers(4, 1, SubMeshConstantBuffer);
        //SetPSConstantBuffers(5, 1, TextureConstantBuffer);
        SetVSConstantBuffers(6, 1, CameraConstantBuffer);
    }
}

void FRenderer::ResetVertexShader()
{
    SetVertexShader(nullptr);
    VertexShader->Release();
}

void FRenderer::ResetPixelShader()
{
    SetPixelShader(nullptr);
    PixelShader->Release();
}

void FRenderer::CreateVertexShader(const FWString& filename, const FString& funcname, const FString& version)
{
    // ���� �߻��� ���ɼ��� ����
    if (Graphics == nullptr)
        assert(0);
    if (VertexShader != nullptr)
        ResetVertexShader();
    if (InputLayout != nullptr)
        InputLayout->Release();
    ID3DBlob* vertexshaderCSO;

    D3DCompileFromFile(filename.c_str(), nullptr, nullptr, *funcname, *version, 0, 0, &vertexshaderCSO, nullptr);
    Graphics->Device->CreateVertexShader(vertexshaderCSO->GetBufferPointer(), vertexshaderCSO->GetBufferSize(), nullptr, &VertexShader);
    vertexshaderCSO->Release();
}

void FRenderer::CreatePixelShader(const FWString& filename, const FString& funcname, const FString& version)
{
    // ���� �߻��� ���ɼ��� ����
    if (Graphics == nullptr)
        assert(0);
    if (VertexShader != nullptr)
        ResetVertexShader();
    ID3DBlob* pixelshaderCSO;
    D3DCompileFromFile(filename.c_str(), nullptr, nullptr, *funcname, *version, 0, 0, &pixelshaderCSO, nullptr);
    Graphics->Device->CreatePixelShader(pixelshaderCSO->GetBufferPointer(), pixelshaderCSO->GetBufferSize(), nullptr, &PixelShader);

    pixelshaderCSO->Release();
}

void FRenderer::ChangeViewMode(EViewModeIndex evi)
{
    switch (evi)
    {
    case EViewModeIndex::VMI_Lit:
        //UpdateLitUnlitConstant(1);
        //break;
    case EViewModeIndex::VMI_Wireframe:
    case EViewModeIndex::VMI_Unlit:
        UpdateLitUnlitConstant(0);
        break;
    }
}

// void FRenderer::RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices) const
// {
//     UINT offset = 0;
//     Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &Stride, &offset);
//     Graphics->DeviceContext->Draw(numVertices, 0);
// }
//
// void FRenderer::RenderPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices) const
// {
//     UINT offset = 0;
//     Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &Stride, &offset);
//     Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
//
//     Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
// }

void FRenderer::RenderPrimitive(OBJ::FStaticMeshRenderData* renderData, TArray<FStaticMaterial*> materials, TArray<UMaterial*> overrideMaterial, int selectedSubMeshIndex = -1)
{
    if (renderData == nullptr || renderData->IndexBuffer == nullptr)
        return;
    
    
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &renderData->VertexBuffer, &Stride, &offset);
    
    Graphics->DeviceContext->IASetIndexBuffer(renderData->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    if (renderData->MaterialSubsets.Num() == 0)
    {
        // no submesh
        Graphics->DeviceContext->DrawIndexed(renderData->Indices.Num(), 0, 0);
    }

    for (int subMeshIndex = 0; subMeshIndex < renderData->MaterialSubsets.Num(); subMeshIndex++)
    {
        int materialIndex = renderData->MaterialSubsets[subMeshIndex].MaterialIndex;

        subMeshIndex == selectedSubMeshIndex ? UpdateSubMeshConstant(true) : UpdateSubMeshConstant(false);

        overrideMaterial[materialIndex] != nullptr ? 
            UpdateMaterial(overrideMaterial[materialIndex]->GetMaterialInfo()) : UpdateMaterial(materials[materialIndex]->Material->GetMaterialInfo());


        // 동일한 버퍼에 Sub Mesh의 IndexStart, IndexCount만 다르게 하고 있다.
        
        // index draw
        uint64 startIndex = renderData->MaterialSubsets[subMeshIndex].IndexStart;
        uint64 indexCount = renderData->MaterialSubsets[subMeshIndex].IndexCount;
        Graphics->DeviceContext->DrawIndexed(indexCount, startIndex, 0);
    }
}

void FRenderer::RenderTexturedModelPrimitive(
    ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices, ID3D11ShaderResourceView* InTextureSRV,
    ID3D11SamplerState* InSamplerState
)
{
    if (!InTextureSRV || !InSamplerState)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "SRV, Sampler Error");
    }
    if (numIndices <= 0)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "numIndices Error");
    }
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &Stride, &offset);
    Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    //Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    SetPSTextureSRV(0, 1, InTextureSRV);
    SetPSSamplerState(0 ,1, InSamplerState);

    Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
}

ID3D11Buffer* FRenderer::CreateVertexBuffer(FVertexSimple* vertices, UINT byteWidth) const
{
    // 2. Create a vertex buffer
    D3D11_BUFFER_DESC vertexbufferdesc = {};
    vertexbufferdesc.ByteWidth = byteWidth;
    vertexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated 
    vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexbufferSRD = {vertices};

    ID3D11Buffer* vertexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD, &vertexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "VertexBuffer Creation faild");
    }
    return vertexBuffer;
}

ID3D11Buffer* FRenderer::CreateVertexBuffer(const TArray<FVertexSimple>& vertices, UINT byteWidth) const
{
    D3D11_BUFFER_DESC vertexbufferdesc = {};
    vertexbufferdesc.ByteWidth = byteWidth;
    vertexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated 
    vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexbufferSRD = {};
    vertexbufferSRD.pSysMem = vertices.GetData();

    ID3D11Buffer* vertexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD, &vertexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "VertexBuffer Creation faild");
    }
    return vertexBuffer;
}

ID3D11Buffer* FRenderer::CreateIndexBuffer(uint32* indices, UINT byteWidth) const
{
    D3D11_BUFFER_DESC indexbufferdesc = {};              // buffer�� ����, �뵵 ���� ����
    indexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE;       // immutable: gpu�� �б� �������� ������ �� �ִ�.
    indexbufferdesc.BindFlags = D3D11_BIND_INDEX_BUFFER; // index buffer�� ����ϰڴ�.
    indexbufferdesc.ByteWidth = byteWidth;               // buffer ũ�� ����
    indexbufferdesc.CPUAccessFlags = 0;
    indexbufferdesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA indexbufferSRD = {indices};

    ID3D11Buffer* indexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&indexbufferdesc, &indexbufferSRD, &indexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "IndexBuffer Creation faild");
    }
    return indexBuffer;
}

ID3D11Buffer* FRenderer::CreateIndexBuffer(const TArray<uint32>& indices, UINT byteWidth) const
{
    D3D11_BUFFER_DESC indexbufferdesc = {};              // buffer�� ����, �뵵 ���� ����
    indexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE;       // immutable: gpu�� �б� �������� ������ �� �ִ�.
    indexbufferdesc.BindFlags = D3D11_BIND_INDEX_BUFFER; // index buffer�� ����ϰڴ�.
    indexbufferdesc.ByteWidth = byteWidth;               // buffer ũ�� ����
    indexbufferdesc.CPUAccessFlags = 0;
    indexbufferdesc.MiscFlags = 0;
    
    D3D11_SUBRESOURCE_DATA indexbufferSRD;
    indexbufferSRD.pSysMem = indices.GetData();

    ID3D11Buffer* indexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&indexbufferdesc, &indexbufferSRD, &indexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "IndexBuffer Creation faild");
    }
    return indexBuffer;
}

void FRenderer::ReleaseBuffer(ID3D11Buffer*& Buffer) const
{
    if (Buffer)
    {
        Buffer->Release();
        Buffer = nullptr;
    }
}

void FRenderer::CreateConstantBuffer()
{
    D3D11_BUFFER_DESC constantbufferdesc = {};
    constantbufferdesc.ByteWidth = sizeof(FConstants) + 0xf & 0xfffffff0;
    constantbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
    constantbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constantbufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    Graphics->Device->CreateBuffer(&constantbufferdesc, nullptr, &ConstantBuffer);

    constantbufferdesc.ByteWidth = sizeof(FSubUVConstant) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&constantbufferdesc, nullptr, &SubUVConstantBuffer);

    constantbufferdesc.ByteWidth = sizeof(FGridParameters) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&constantbufferdesc, nullptr, &GridConstantBuffer);

    constantbufferdesc.ByteWidth = sizeof(FPrimitiveCounts) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&constantbufferdesc, nullptr, &LinePrimitiveBuffer);

    constantbufferdesc.ByteWidth = sizeof(FMaterialConstants) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&constantbufferdesc, nullptr, &MaterialConstantBuffer);
    
    // constantbufferdesc.ByteWidth = sizeof(FSubMeshConstants) + 0xf & 0xfffffff0;
    // Graphics->Device->CreateBuffer(&constantbufferdesc, nullptr, &SubMeshConstantBuffer);

    // constantbufferdesc.ByteWidth = sizeof(FTextureConstants) + 0xf & 0xfffffff0;
    // Graphics->Device->CreateBuffer(&constantbufferdesc, nullptr, &TextureConstantBuffer);

    constantbufferdesc.ByteWidth = sizeof(FConstants) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&constantbufferdesc, nullptr, &CameraConstantBuffer);
}

void FRenderer::CreateLightingConstantBuffer()
{
    D3D11_BUFFER_DESC constantbufferdesc = {};
    constantbufferdesc.ByteWidth = sizeof(FLighting);
    constantbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
    constantbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constantbufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Graphics->Device->CreateBuffer(&constantbufferdesc, nullptr, &LightingConstantBuffer);
}

void FRenderer::CreateLitUnlitBuffer()
{
    D3D11_BUFFER_DESC constantbufferdesc = {};
    constantbufferdesc.ByteWidth = sizeof(FLitUnlitConstants);
    constantbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
    constantbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constantbufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Graphics->Device->CreateBuffer(&constantbufferdesc, nullptr, &FlagConstantBuffer);
}

ID3D11Buffer* FRenderer::CreateBuffer(void* Data, uint32 ByteWidth, D3D11_USAGE Usage, D3D11_CPU_ACCESS_FLAG Flag, D3D11_BIND_FLAG BindFlag)
{
    D3D11_BUFFER_DESC bufferdesc = {};
    bufferdesc.ByteWidth = ByteWidth;
    bufferdesc.Usage = Usage; // will never be updated 
    bufferdesc.BindFlags = BindFlag;
    bufferdesc.CPUAccessFlags = Flag;

    D3D11_SUBRESOURCE_DATA vertexbufferSRD = {Data};

    ID3D11Buffer* vertexBuffer;
    HRESULT hr;
    if (Data == nullptr)
        hr = Graphics->Device->CreateBuffer(&bufferdesc, nullptr, &vertexBuffer);
    else
        hr = Graphics->Device->CreateBuffer(&bufferdesc, &vertexbufferSRD, &vertexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "VertexBuffer Creation faild");
    }
    return vertexBuffer;
}

void FRenderer::ReleaseConstantBuffer()
{
    if (ConstantBuffer)
    {
        ConstantBuffer->Release();
        ConstantBuffer = nullptr;
    }

    if (LightingConstantBuffer)
    {
        LightingConstantBuffer->Release();
        LightingConstantBuffer = nullptr;
    }

    if (FlagConstantBuffer)
    {
        FlagConstantBuffer->Release();
        FlagConstantBuffer = nullptr;
    }

    if (MaterialConstantBuffer)
    {
        MaterialConstantBuffer->Release();
        MaterialConstantBuffer = nullptr;
    }

    if (SubMeshConstantBuffer)
    {
        SubMeshConstantBuffer->Release();
        SubMeshConstantBuffer = nullptr;
    }

    if (TextureConstantBuffer)
    {
        TextureConstantBuffer->Release();
        TextureConstantBuffer = nullptr;
    }

    if (CameraConstantBuffer)
    {
        CameraConstantBuffer->Release();
        CameraConstantBuffer = nullptr;
    }
}

void FRenderer::UpdateLightConstantBuffer() const
{
    if (!LightingConstantBuffer) return;
    
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    Graphics->DeviceContext->Map(LightingConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    {
        FLighting* constants = static_cast<FLighting*>(mappedResource.pData);
        constants->lightDirX = 1.0f; // ��: ���� ������ �Ʒ��� �������� ���
        constants->lightDirY = 1.0f; // ��: ���� ������ �Ʒ��� �������� ���
        constants->lightDirZ = 1.0f; // ��: ���� ������ �Ʒ��� �������� ���
        constants->lightColorX = 1.0f;
        constants->lightColorY = 1.0f;
        constants->lightColorZ = 1.0f;
        constants->AmbientFactor = 0.06f;
    }
    Graphics->DeviceContext->Unmap(LightingConstantBuffer, 0);
}

// void FRenderer::UpdateConstant(const FMatrix& MVP, const FMatrix& NormalMatrix, FVector4 UUIDColor, bool IsSelected) const
// {
//     if (ConstantBuffer)
//     {
//         D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����
//
//         Graphics->DeviceContext->Map(ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
//         {
//             FConstants* constants = static_cast<FConstants*>(ConstantBufferMSR.pData);
//             constants->MVP = MVP;
//             // constants->ModelMatrixInverseTranspose = NormalMatrix;
//             // constants->UUIDColor = UUIDColor;
//             // constants->IsSelected = IsSelected;
//         }
//         Graphics->DeviceContext->Unmap(ConstantBuffer, 0); // GPU�� �ٽ� ��밡���ϰ� �����
//     }
// }

void FRenderer::UpdateConstant(const FMatrix& MVP) const
{
    if (ConstantBuffer == nullptr)
        return;
    
    D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

    Graphics->DeviceContext->Map(ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
    {
        FConstants* constants = static_cast<FConstants*>(ConstantBufferMSR.pData);
        constants->MVP = MVP;
    }
    Graphics->DeviceContext->Unmap(ConstantBuffer, 0); // GPU�� �ٽ� ��밡���ϰ� �����
}

void FRenderer::UpdateMaterial(const FObjMaterialInfo& MaterialInfo)
{
    if (MaterialConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        Graphics->DeviceContext->Map(MaterialConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        {
            FMaterialConstants* constants = static_cast<FMaterialConstants*>(ConstantBufferMSR.pData);
            constants->DiffuseColor = MaterialInfo.Diffuse;
            // constants->TransparencyScalar = MaterialInfo.TransparencyScalar;
            // constants->AmbientColor = MaterialInfo.Ambient;
            // constants->DensityScalar = MaterialInfo.DensityScalar;
            // constants->SpecularColor = MaterialInfo.Specular;
            // constants->SpecularScalar = MaterialInfo.SpecularScalar;
            // constants->EmmisiveColor = MaterialInfo.Emissive;
        }
        Graphics->DeviceContext->Unmap(MaterialConstantBuffer, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }

    if (MaterialInfo.bHasTexture == true)
    {
        std::shared_ptr<FTexture> texture = FEngineLoop::resourceMgr.GetTexture(MaterialInfo.DiffuseTexturePath);

        SetPSTextureSRV(0, 1, texture->TextureSRV);
        SetPSSamplerState(0, 1, texture->SamplerState);
    }
    else
    {
        SetPSTextureSRV(0, 1, nullptr);
        SetPSSamplerState(0, 1, nullptr);
    }
}

void FRenderer::UpdateLitUnlitConstant(int32 isLit)
{
    if (bFlagIsLit != isLit)
        return;

    bFlagIsLit = isLit;
    
    if (FlagConstantBuffer == nullptr)
        return;
    
    D3D11_MAPPED_SUBRESOURCE constantbufferMSR; // GPU �� �޸� �ּ� ����
    Graphics->DeviceContext->Map(FlagConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
    auto constants = static_cast<FLitUnlitConstants*>(constantbufferMSR.pData); //GPU �޸� ���� ����
    {
        constants->isLit = isLit;
    }
    Graphics->DeviceContext->Unmap(FlagConstantBuffer, 0);
}

void FRenderer::UpdateSubMeshConstant(bool isSelected) const
{
    if (SubMeshConstantBuffer) {
        D3D11_MAPPED_SUBRESOURCE constantbufferMSR; // GPU �� �޸� �ּ� ����
        Graphics->DeviceContext->Map(SubMeshConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
        FSubMeshConstants* constants = (FSubMeshConstants*)constantbufferMSR.pData; //GPU �޸� ���� ����
        {
            constants->isSelectedSubMesh = isSelected;
        }
        Graphics->DeviceContext->Unmap(SubMeshConstantBuffer, 0);
    }
}

void FRenderer::UpdateTextureConstant(float UOffset, float VOffset)
{
    if (TextureConstantBuffer) {
        D3D11_MAPPED_SUBRESOURCE constantbufferMSR; // GPU �� �޸� �ּ� ����
        Graphics->DeviceContext->Map(TextureConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
        FTextureConstants* constants = (FTextureConstants*)constantbufferMSR.pData; //GPU �޸� ���� ����
        {
            constants->UOffset = UOffset;
            constants->VOffset = VOffset;
        }
        Graphics->DeviceContext->Unmap(TextureConstantBuffer, 0);
    }
}

void FRenderer::UpdateCameraConstant(FEditorViewportClient* ActiveViewport) const
{
    FMatrix VP = ActiveViewport->GetViewMatrix() * ActiveViewport->GetProjectionMatrix();
    
    if (CameraConstantBuffer == nullptr)
        return;
    
    D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

    Graphics->DeviceContext->Map(CameraConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
    {
        FConstants* constants = static_cast<FConstants*>(ConstantBufferMSR.pData);
        constants->MVP = VP;
    }
    Graphics->DeviceContext->Unmap(CameraConstantBuffer, 0); // GPU�� �ٽ� ��밡���ϰ� �����
}

void FRenderer::CreateTextureShader()
{
    ID3DBlob* vertextextureshaderCSO;
    ID3DBlob* pixeltextureshaderCSO;

    HRESULT hr;
    hr = D3DCompileFromFile(L"Shaders/VertexTextureShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &vertextextureshaderCSO, nullptr);
    if (FAILED(hr))
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "VertexShader Error");
    }
    Graphics->Device->CreateVertexShader(
        vertextextureshaderCSO->GetBufferPointer(), vertextextureshaderCSO->GetBufferSize(), nullptr, &VertexTextureShader
    );

    hr = D3DCompileFromFile(L"Shaders/PixelTextureShader.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &pixeltextureshaderCSO, nullptr);
    if (FAILED(hr))
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "PixelShader Error");
    }
    Graphics->Device->CreatePixelShader(
        pixeltextureshaderCSO->GetBufferPointer(), pixeltextureshaderCSO->GetBufferSize(), nullptr, &PixelTextureShader
    );

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), vertextextureshaderCSO->GetBufferPointer(), vertextextureshaderCSO->GetBufferSize(), &TextureInputLayout
    );

    //�ڷᱸ�� ���� �ʿ�
    TextureStride = sizeof(FVertexTexture);
    vertextextureshaderCSO->Release();
    pixeltextureshaderCSO->Release();
}

void FRenderer::ReleaseTextureShader()
{
    if (TextureInputLayout)
    {
        TextureInputLayout->Release();
        TextureInputLayout = nullptr;
    }

    if (PixelTextureShader)
    {
        PixelTextureShader->Release();
        PixelTextureShader = nullptr;
    }

    if (VertexTextureShader)
    {
        VertexTextureShader->Release();
        VertexTextureShader = nullptr;
    }
    if (SubUVConstantBuffer)
    {
        SubUVConstantBuffer->Release();
        SubUVConstantBuffer = nullptr;
    }
    if (ConstantBuffer)
    {
        ConstantBuffer->Release();
        ConstantBuffer = nullptr;
    }

    if (CameraConstantBuffer)
    {
        CameraConstantBuffer->Release();
        CameraConstantBuffer = nullptr;
    }
}

void FRenderer::PrepareTextureShader()
{
    SetVertexShader(VertexTextureShader);
    SetPixelShader(PixelTextureShader);
    SetInputLayout(TextureInputLayout);

    //�ؽ��Ŀ� ConstantBuffer �߰��ʿ��Ҽ���
    if (ConstantBuffer)
    {
        SetVSConstantBuffers(0, 1, ConstantBuffer);
    }
}

ID3D11Buffer* FRenderer::CreateVertexTextureBuffer(FVertexTexture* vertices, UINT byteWidth) const
{
    // 2. Create a vertex buffer
    D3D11_BUFFER_DESC vertexbufferdesc = {};
    vertexbufferdesc.ByteWidth = byteWidth;
    vertexbufferdesc.Usage = D3D11_USAGE_DYNAMIC; // will never be updated 
    vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    //D3D11_SUBRESOURCE_DATA vertexbufferSRD = { vertices };

    ID3D11Buffer* vertexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&vertexbufferdesc, nullptr, &vertexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "VertexBuffer Creation faild");
    }
    return vertexBuffer;
}

ID3D11Buffer* FRenderer::CreateIndexTextureBuffer(uint32* indices, UINT byteWidth) const
{
    D3D11_BUFFER_DESC indexbufferdesc = {};
    indexbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
    indexbufferdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexbufferdesc.ByteWidth = byteWidth;
    indexbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ID3D11Buffer* indexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&indexbufferdesc, nullptr, &indexBuffer);
    if (FAILED(hr))
    {
        return nullptr;
    }
    return indexBuffer;
}

void FRenderer::RenderTexturePrimitive(
    ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices, ID3D11ShaderResourceView* _TextureSRV,
    ID3D11SamplerState* _SamplerState
)
{
    if (!_TextureSRV || !_SamplerState)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "SRV, Sampler Error");
    }
    if (numIndices <= 0)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "numIndices Error");
    }
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &TextureStride, &offset);
    Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    SetPSTextureSRV(0, 1, _TextureSRV);
    SetPSSamplerState(0, 1, _SamplerState);

    Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
}

//��Ʈ ��ġ������
void FRenderer::RenderTextPrimitive(
    ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11ShaderResourceView* _TextureSRV, ID3D11SamplerState* _SamplerState
)
{
    if (!_TextureSRV || !_SamplerState)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "SRV, Sampler Error");
    }
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &TextureStride, &offset);

    SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    SetPSTextureSRV(0, 1, _TextureSRV);
    SetPSSamplerState(0, 1, _SamplerState);
    

    // ��ο� ȣ�� (6���� �ε��� ���)
    Graphics->DeviceContext->Draw(numVertices, 0);
}


ID3D11Buffer* FRenderer::CreateVertexBuffer(FVertexTexture* vertices, UINT byteWidth) const
{
    // 2. Create a vertex buffer
    D3D11_BUFFER_DESC vertexbufferdesc = {};
    vertexbufferdesc.ByteWidth = byteWidth;
    vertexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated 
    vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexbufferSRD = {vertices};

    ID3D11Buffer* vertexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD, &vertexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "VertexBuffer Creation faild");
    }
    return vertexBuffer;
}

void FRenderer::UpdateSubUVConstant(float _indexU, float _indexV) const
{
    if (SubUVConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE constantbufferMSR; // GPU�� �޸� �ּ� ����

        Graphics->DeviceContext->Map(SubUVConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR); // update constant buffer every frame
        auto constants = static_cast<FSubUVConstant*>(constantbufferMSR.pData);                               //GPU �޸� ���� ����
        {
            constants->indexU = _indexU;
            constants->indexV = _indexV;
        }
        Graphics->DeviceContext->Unmap(SubUVConstantBuffer, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

void FRenderer::PrepareSubUVConstant()
{
    if (SubUVConstantBuffer)
    {
        SetVSConstantBuffers(1, 1, SubUVConstantBuffer);
        SetPSConstantBuffers(1, 1, SubUVConstantBuffer);
    }
}

void FRenderer::PrepareLineShader()
{
    // ���̴��� �Է� ���̾ƿ� ����
    
    SetVertexShader(VertexLineShader);
    SetPixelShader(PixelLineShader);

    // ��� ���� ���ε�: 
    // - MatrixBuffer�� register(b0)��, Vertex Shader�� ���ε�
    // - GridConstantBuffer�� register(b1)��, Vertex�� Pixel Shader�� ���ε� (�ȼ� ���̴��� �ʿ信 ����)
    if (ConstantBuffer && GridConstantBuffer)
    {
        SetVSConstantBuffers(0, 1, ConstantBuffer);     // MatrixBuffer (b0)
        SetVSConstantBuffers(1, 1, GridConstantBuffer); // GridParameters (b1)
        SetPSConstantBuffers(1, 1, GridConstantBuffer);
        SetVSConstantBuffers(3, 1, LinePrimitiveBuffer);
        Graphics->DeviceContext->VSSetShaderResources(2, 1, &pBBSRV);
        Graphics->DeviceContext->VSSetShaderResources(3, 1, &pConeSRV);
        Graphics->DeviceContext->VSSetShaderResources(4, 1, &pOBBSRV);
    }
}

void FRenderer::CreateLineShader()
{
    ID3DBlob* VertexShaderLine;
    ID3DBlob* PixelShaderLine;

    HRESULT hr;
    hr = D3DCompileFromFile(L"Shaders/ShaderLine.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &VertexShaderLine, nullptr);
    if (FAILED(hr))
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "VertexShader Error");
    }
    Graphics->Device->CreateVertexShader(VertexShaderLine->GetBufferPointer(), VertexShaderLine->GetBufferSize(), nullptr, &VertexLineShader);

    hr = D3DCompileFromFile(L"Shaders/ShaderLine.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &PixelShaderLine, nullptr);
    if (FAILED(hr))
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "PixelShader Error");
    }
    Graphics->Device->CreatePixelShader(PixelShaderLine->GetBufferPointer(), PixelShaderLine->GetBufferSize(), nullptr, &PixelLineShader);


    VertexShaderLine->Release();
    PixelShaderLine->Release();
}

void FRenderer::ReleaseLineShader() const
{
    if (GridConstantBuffer) GridConstantBuffer->Release();
    if (LinePrimitiveBuffer) LinePrimitiveBuffer->Release();
    if (VertexLineShader) VertexLineShader->Release();
    if (PixelLineShader) PixelLineShader->Release();
}

ID3D11Buffer* FRenderer::CreateStaticVerticesBuffer() const
{
    FSimpleVertex vertices[2]{{0}, {0}};

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA vbInitData = {};
    vbInitData.pSysMem = vertices;
    ID3D11Buffer* pVertexBuffer = nullptr;
    HRESULT hr = Graphics->Device->CreateBuffer(&vbDesc, &vbInitData, &pVertexBuffer);
    return pVertexBuffer;
}

ID3D11Buffer* FRenderer::CreateBoundingBoxBuffer(UINT numBoundingBoxes) const
{
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC; // ���� ������Ʈ�� ��� DYNAMIC, �׷��� ������ DEFAULT
    bufferDesc.ByteWidth = sizeof(FBoundingBox) * numBoundingBoxes;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(FBoundingBox);

    ID3D11Buffer* BoundingBoxBuffer = nullptr;
    Graphics->Device->CreateBuffer(&bufferDesc, nullptr, &BoundingBoxBuffer);
    return BoundingBoxBuffer;
}

ID3D11Buffer* FRenderer::CreateOBBBuffer(UINT numBoundingBoxes) const
{
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC; // ���� ������Ʈ�� ��� DYNAMIC, �׷��� ������ DEFAULT
    bufferDesc.ByteWidth = sizeof(FOBB) * numBoundingBoxes;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(FOBB);

    ID3D11Buffer* BoundingBoxBuffer = nullptr;
    Graphics->Device->CreateBuffer(&bufferDesc, nullptr, &BoundingBoxBuffer);
    return BoundingBoxBuffer;
}

ID3D11Buffer* FRenderer::CreateConeBuffer(UINT numCones) const
{
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = sizeof(FCone) * numCones;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(FCone);

    ID3D11Buffer* ConeBuffer = nullptr;
    Graphics->Device->CreateBuffer(&bufferDesc, nullptr, &ConeBuffer);
    return ConeBuffer;
}

ID3D11ShaderResourceView* FRenderer::CreateBoundingBoxSRV(ID3D11Buffer* pBoundingBoxBuffer, UINT numBoundingBoxes)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN; // ����ü ������ ��� UNKNOWN
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Buffer.NumElements = numBoundingBoxes;


    Graphics->Device->CreateShaderResourceView(pBoundingBoxBuffer, &srvDesc, &pBBSRV);
    return pBBSRV;
}

ID3D11ShaderResourceView* FRenderer::CreateOBBSRV(ID3D11Buffer* pBoundingBoxBuffer, UINT numBoundingBoxes)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN; // ����ü ������ ��� UNKNOWN
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Buffer.NumElements = numBoundingBoxes;
    Graphics->Device->CreateShaderResourceView(pBoundingBoxBuffer, &srvDesc, &pOBBSRV);
    return pOBBSRV;
}

ID3D11ShaderResourceView* FRenderer::CreateConeSRV(ID3D11Buffer* pConeBuffer, UINT numCones)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN; // ����ü ������ ��� UNKNOWN
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Buffer.NumElements = numCones;


    Graphics->Device->CreateShaderResourceView(pConeBuffer, &srvDesc, &pConeSRV);
    return pConeSRV;
}

void FRenderer::CreateBatchRenderCache()
{
    for (auto& [MaterialName, BatchRenderTargetContext] : BatchRenderTargets)
    {
        if(!BatchRenderTargetContext.bIsDirty)
            return;
        BatchRenderTargetContext.bIsDirty = false;

        if (!CachedBuffers.Contains(MaterialName))
            CachedBuffers.Add(MaterialName, TMap<uint32, TPair<ID3D11Buffer*, TPair<uint32, ID3D11Buffer*>>>());

#if UseBufferDynamic
        {}
#else
        {
            for (const auto& [BufferIndex, BufferInfo] : CachedBuffers[MaterialName])
            {
                if (BufferInfo.Key)
                    BufferInfo.Key->Release();
                if (BufferInfo.Value.Value)
                    BufferInfo.Value.Value->Release();
            } 
        
            CachedBuffers[MaterialName].Empty();
        }
#endif
        
        int MeshIndex = 0;
        uint32 BufferIndex = 0;
        while (BatchRenderTargetContext.StaticMeshes.Num() > MeshIndex)
        {
            uint32 VertexOffset = 0;
            TArray<FVertexSimple> VertexData;
            TArray<uint32> IndexData;


            uint32 TotalVertexDataSize = 0;
            uint32 TotalIndexDataSize = 0;

            for ( const auto& [MeshIndex, MeshComp]: BatchRenderTargetContext.StaticMeshes ) {
                const OBJ::FStaticMeshRenderData* renderData = MeshComp->GetStaticMesh()->GetRenderData();
                TotalVertexDataSize += MeshComp->Vertices.Num();
                TotalIndexDataSize += renderData->MaterialSubsets[MeshIndex].IndexCount;
            }

            VertexData.Reserve(TotalVertexDataSize);
            VertexData.Reserve(TotalIndexDataSize);

            for (; MeshIndex < BatchRenderTargetContext.StaticMeshes.Num(); MeshIndex++)
            {
                UStaticMeshComponent* StaticMeshComponent = BatchRenderTargetContext.StaticMeshes[MeshIndex].Value;
                const uint32 SubMeshIndex = BatchRenderTargetContext.StaticMeshes[MeshIndex].Key;
                const OBJ::FStaticMeshRenderData* renderData = StaticMeshComponent->GetStaticMesh()->GetRenderData();
                const uint32 IndexBufferStartIndex = renderData->MaterialSubsets[SubMeshIndex].IndexStart;
                const uint32 IndexCount = renderData->MaterialSubsets[SubMeshIndex].IndexCount;

                // LOD TODO: Vertex 다른 거로 넣어주기 변경
                const TArray<FVertexSimple> &Vertices = StaticMeshComponent->Vertices;

                if ((VertexOffset + Vertices.Num()) * sizeof(FVertexSimple) > MaxBufferSize)
                {
                    MeshIndex--;
                    break;
                }

                // LOD TODO: IndexData 다른 거로 넣어주기 변경
                const TArray<uint32>& Indices = renderData->Indices;
                if ((IndexData.Num() + Indices.Num()) * sizeof(uint32) > MaxBufferSize)
                {
                    MeshIndex--;
                    break;
                }
                
                // UISOO TODO: 사용하지 않는 Vertex 없애고 Indexing 당기기
                VertexData.Append(Vertices);
                const uint32 IndexSizeBeforeAppend = IndexData.Num();
                const uint32 IndexBufferEndIndex = IndexBufferStartIndex + IndexCount;
                IndexData.Append(const_cast<uint32*>(Indices.GetData() + IndexBufferStartIndex), IndexCount);
                for (int i = IndexBufferStartIndex; i < IndexBufferEndIndex; ++i)
                {
                    //IndexData.Add(Indices[i] + VertexOffset);
                    IndexData[IndexSizeBeforeAppend + i] += VertexOffset;
                }
                
                VertexOffset += Vertices.Num();
            }

            uint32 VertexDataSize = sizeof(FVertexSimple) * VertexData.Num();
        
            uint32 IndexDataSize = sizeof(uint32) * IndexData.Num();

#if UseBufferDynamic
            UpdateOrCreateBuffer(MaterialName, BufferIndex, VertexData.GetData(), VertexDataSize, IndexData.GetData(), IndexDataSize, IndexData.Num(), MaxBufferSize);
#else
            ID3D11Buffer* VertexBuffer = CreateVertexBuffer(VertexData.GetData(), VertexDataSize);
            ID3D11Buffer* IndexBuffer = CreateIndexBuffer(IndexData.GetData(), IndexDataSize);
            CachedBuffers[MaterialName].Add(BufferIndex, {VertexBuffer, {IndexDataSize, IndexBuffer}});
#endif
            

            MeshIndex++;
            BufferIndex++;
        }
#if UseBufferDynamic
        ReleaseUnUsedBatchBuffer(MaterialName, BufferIndex);
#else
#endif
    }
}

void FRenderer::UpdateOrCreateBuffer(const FString& MaterialName, uint32 BufferIndex, FVertexSimple* VertexData, uint32 VertexDataSize, void* IndexData, uint32 IndexDataSize, uint32 IndexDataCount, uint32 BufferSize)
{
    if (!CachedBuffers[MaterialName].Contains(BufferIndex))
    {
        ID3D11Buffer* VertexBuffer = CreateBuffer(nullptr, BufferSize, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, D3D11_BIND_VERTEX_BUFFER);
        ID3D11Buffer* IndexBuffer = CreateBuffer(nullptr, BufferSize, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, D3D11_BIND_INDEX_BUFFER);
        CachedBuffers[MaterialName].Add(BufferIndex, {VertexBuffer, {0, IndexBuffer}});
    }

    CachedBuffers[MaterialName][BufferIndex].Value.Key = IndexDataCount;
    
    ID3D11Buffer* VertexBuffer = CachedBuffers[MaterialName][BufferIndex].Key;
    // 기존 버퍼 업데이트
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = Graphics->DeviceContext->Map(VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(hr))
    {
        //reinterpret_cast<FOBB*>(mappedResource.pData);
        memcpy(mappedResource.pData, VertexData, VertexDataSize);
        Graphics->DeviceContext->Unmap(VertexBuffer, 0);
    }
    else
    {
        UE_LOG(LogLevel::Warning, "Failed to map vertex buffer for update.");
    }

    ID3D11Buffer* IndexBuffer = CachedBuffers[MaterialName][BufferIndex].Key;
    // 기존 버퍼 업데이트
    D3D11_MAPPED_SUBRESOURCE mappedResourceIndex;
    hr = Graphics->DeviceContext->Map(IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResourceIndex);
    if (SUCCEEDED(hr))
    {
        memcpy(mappedResourceIndex.pData, IndexData, IndexDataSize);
        Graphics->DeviceContext->Unmap(IndexBuffer, 0);
    }
    else
    {
        UE_LOG(LogLevel::Warning, "Failed to map vertex buffer for update.");
    }
}

void FRenderer::BakeBatchRenderBuffer()
{
    // 액터 조회
    TSet<AActor*> Actors = GEngineLoop.GetWorld()->GetActors();


    TArray<UStaticMeshComponent*> components;
    GEngineLoop.GetWorld()->GetOcTree().PrepareCull([&](const FOctreeElement<UStaticMeshComponent>& element)
    {
        components.Add(element.element);
    });
    
    for (auto* pStaticMeshComp : components)
    {
        for (uint32 i = 0; i < pStaticMeshComp->GetNumMaterials(); ++i)
        {
            UMaterial* Material = pStaticMeshComp->GetMaterial(i);
            // LOD TODO: Texture 다른 거로 넣어주기
            const FString& MTLName = Material->GetMaterialInfo().MTLName;
            if (!BatchRenderTargets.Contains(MTLName))
            {
                BatchRenderTargets.Add(MTLName, BatchRenderTargetContext());
                BatchRenderTargets[MTLName].bIsDirty = true;
            }
            BatchRenderTargets[MTLName].StaticMeshes.Add({ i, pStaticMeshComp });
        }
    }

    // 각 Material에 대해 LOD별 누적 데이터를 별도로 관리
    for (auto& [MaterialName, BatchRenderTargetContext] : BatchRenderTargets)
    {
        // 예제에서는 LOD 레벨을 3으로 가정 (필요시 동적 결정 가능)
        const int32 LODCount = 3;
        BakedLODBuffers[MaterialName].Empty();
        BakedLODBuffers[MaterialName].SetNum(LODCount);

        // 각 LOD별 누적 정점/인덱스 데이터와 누적 버텍스 오프셋을 초기화
        TArray<TArray<FVertexSimple>> AccumVertexData;
        TArray<TArray<uint32>> AccumIndexData;
        TArray<uint32> AccumVertexOffset;
        AccumVertexData.SetNum(LODCount);
        AccumIndexData.SetNum(LODCount);
        AccumVertexOffset.Init(0, LODCount);

        // 해당 Material에 속한 StaticMeshComponent들의 데이터를 누적합니다.
        for (auto& [MaterialIndex, pStaticMeshComp] : BatchRenderTargetContext.StaticMeshes)
        {
            // 각 메시의 LOD 데이터를 가져옴
            auto LODRenderDatas = pStaticMeshComp->GetStaticMesh()->GetLODDatas();
            const int32 MeshLODCount = LODRenderDatas.Num();
            for (int32 lodLevel = 0; lodLevel < MeshLODCount && lodLevel < LODCount; ++lodLevel)
            {
                // 해당 LOD의 정점과 인덱스 데이터
                const TArray<FVertexSimple>& Vertices = pStaticMeshComp->LODVertices[lodLevel];
                TArray<uint32> Indices = pStaticMeshComp->GetStaticMesh()->GetRenderData(lodLevel)->Indices;

                // 인덱스에 현재 누적된 버텍스 오프셋을 적용
                for (int32 i = 0; i < Indices.Num(); ++i)
                {
                    Indices[i] += AccumVertexOffset[lodLevel];
                }

                // 해당 LOD 누적 데이터에 추가
                AccumVertexData[lodLevel].Append(Vertices);
                AccumIndexData[lodLevel].Append(Indices);
                AccumVertexOffset[lodLevel] += Vertices.Num();

                // 누적 데이터의 총 메모리 사용량이 MaxBufferSize를 초과하면 flush
                if (sizeof(FVertexSimple) * AccumVertexData[lodLevel].Num() > MaxBufferSize ||
                    sizeof(uint32) * AccumIndexData[lodLevel].Num() > MaxBufferSize)
                {
                    BakedLODBuffers[MaterialName][lodLevel].VertexBuffer.Add(
                        CreateVertexBuffer(AccumVertexData[lodLevel], sizeof(FVertexSimple) * AccumVertexData[lodLevel].Num()));
                    BakedLODBuffers[MaterialName][lodLevel].IndexBuffer.Add(
                        CreateIndexBuffer(AccumIndexData[lodLevel], sizeof(uint32) * AccumIndexData[lodLevel].Num()));
                    BakedLODBuffers[MaterialName][lodLevel].Stride = sizeof(FVertexSimple);
                    BakedLODBuffers[MaterialName][lodLevel].IndexCount.Add(AccumIndexData[lodLevel].Num());

                    // 해당 LOD의 누적 데이터 초기화
                    AccumVertexData[lodLevel].Empty();
                    AccumIndexData[lodLevel].Empty();
                    AccumVertexOffset[lodLevel] = 0;
                }
            }
        }

        // 모든 메시 처리가 끝난 후, 각 LOD별 남은 데이터를 flush
        for (int32 lodLevel = 0; lodLevel < LODCount; ++lodLevel)
        {
            if (AccumVertexData[lodLevel].Num() > 0 && AccumIndexData[lodLevel].Num() > 0)
            {
                BakedLODBuffers[MaterialName][lodLevel].VertexBuffer.Add(
                    CreateVertexBuffer(AccumVertexData[lodLevel], sizeof(FVertexSimple) * AccumVertexData[lodLevel].Num()));
                BakedLODBuffers[MaterialName][lodLevel].IndexBuffer.Add(
                    CreateIndexBuffer(AccumIndexData[lodLevel], sizeof(uint32) * AccumIndexData[lodLevel].Num()));
                BakedLODBuffers[MaterialName][lodLevel].Stride = sizeof(FVertexSimple);
                BakedLODBuffers[MaterialName][lodLevel].IndexCount.Add(AccumIndexData[lodLevel].Num());
            }
        }
    }
}


void FRenderer::RenderBakedBuffer()
{
    constexpr uint32 stride = sizeof(FVertexSimple);
    constexpr uint32 vertexOffset = 0;

    PrepareShader();

    // MaterialConstant
    if ( MaterialConstantBuffer )
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        Graphics->DeviceContext->Map(MaterialConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        {
            FMaterialConstants* constants = static_cast<FMaterialConstants*>(ConstantBufferMSR.pData);
            constants->DiffuseColor = { 0, 0, 0 };
        }
        Graphics->DeviceContext->Unmap(MaterialConstantBuffer, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }



    // Render
    for ( auto& [MaterialName, BatchRenderTargetContext] : BatchRenderTargets )
    {
        uint32 LODIdx = 0;
        uint32 bufferIdx = 0;
        uint32 offset = 0;
        uint32 length = 0;
        
        // Set Material
        UMaterial* Material = FManagerOBJ::GetMaterial(MaterialName);
        const auto& MaterialInfo = Material->GetMaterialInfo();
        if ( MaterialInfo.bHasTexture == true ) {
            std::shared_ptr<FTexture> texture = FEngineLoop::resourceMgr.GetTexture(MaterialInfo.DiffuseTexturePath);

            SetPSTextureSRV(0, 1, texture->TextureSRV);
            SetPSSamplerState(0, 1, texture->SamplerState);
        } else {
            SetPSTextureSRV(0, 1, nullptr);
            SetPSSamplerState(0, 1, nullptr);
        }

        Graphics->DeviceContext->IASetVertexBuffers(0, 1, &BakedLODBuffers[MaterialName][LODIdx].VertexBuffer[bufferIdx], &stride, &vertexOffset);
        Graphics->DeviceContext->IASetIndexBuffer(BakedLODBuffers[MaterialName][LODIdx].IndexBuffer[bufferIdx], DXGI_FORMAT_R32_UINT, 0);

        auto& Meshes = BatchRenderTargetContext.StaticMeshes;

        for ( int i = 0; i < Meshes.Num(); ++i )
        {
            const UStaticMeshComponent* pStaticMeshComp = Meshes[i].Value;
            const auto& renderDatas = pStaticMeshComp->GetStaticMesh()->GetLODDatas();
            const uint32 lodLevel = pStaticMeshComp->GetLODLevel();
            const uint32 indicesCount = renderDatas[lodLevel]->Indices.Num();
            
            // if next meshcomp is visible
            const bool bIsNextVisible = (i + 1 < Meshes.Num()) && !Meshes[i+1].Value->bIsVisible;

            // if meshcomp is not visible
            if (!pStaticMeshComp->bIsVisible)
            {
                if (length > 0) {
                    Graphics->DeviceContext->DrawIndexed(length, offset, 0);
                }

                offset += length + indicesCount;
                length = 0;
            }
            else
            {
                length += indicesCount;
            }

            // if end of array
            if (offset + length >= BakedLODBuffers[MaterialName][lodLevel].IndexCount[bufferIdx])
            {
                if ( length > 0 )
                    Graphics->DeviceContext->DrawIndexed(length, offset, 0);

                offset = 0;
                length = 0;
                ++bufferIdx;
                if ( BakedLODBuffers[MaterialName][lodLevel].VertexBuffer.Num() > bufferIdx )
                {
                    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &BakedLODBuffers[MaterialName][lodLevel].VertexBuffer[bufferIdx], &stride, &vertexOffset);
                    Graphics->DeviceContext->IASetIndexBuffer(BakedLODBuffers[MaterialName][lodLevel].IndexBuffer[bufferIdx], DXGI_FORMAT_R32_UINT, 0);
                }
            }
        }
    }
}

void FRenderer::ReleaseBakedData()
{
    for ( auto& [_, buffer] : BakedBuffers ) {
        for ( auto& vertexBuffer: buffer.VertexBuffer ) {
            vertexBuffer->Release();
        }
        for ( auto& IndexBuffer : buffer.IndexBuffer ) {
            IndexBuffer->Release();
        }
        buffer.VertexBuffer.Empty();
        buffer.IndexBuffer.Empty();
        buffer.Stride = 0;
        buffer.IndexCount.Empty();
    }


    for ( auto& [_, buffers] : BakedLODBuffers ) {
        for ( auto& buffer: buffers) {
            for ( auto& vertexBuffer : buffer.VertexBuffer ) {
                vertexBuffer->Release();
            }
            for ( auto& IndexBuffer : buffer.IndexBuffer ) {
                IndexBuffer->Release();
            }
            buffer.VertexBuffer.Empty();
            buffer.IndexBuffer.Empty();
            buffer.Stride = 0;
            buffer.IndexCount.Empty();
        }
        buffers.Empty();
    }

    for ( auto& [_, context]: BatchRenderTargets ) {
        context.StaticMeshes.Empty();
    }
}

void FRenderer::ReleaseUnUsedBatchBuffer(const FString& MaterialName, uint32 ReleaseStartBufferIndex)
{
    if (!CachedBuffers.Contains(MaterialName))
    {
        return;
    }

    TArray<uint32> BufferIndexes;
    for (const auto& [bufferIndex, BufferInfo] : CachedBuffers[MaterialName])
    {
        if (ReleaseStartBufferIndex > bufferIndex)
            continue;
        ID3D11Buffer* VertexBuffer = BufferInfo.Key;
        ID3D11Buffer* IndexBuffer = BufferInfo.Value.Value;

        if (VertexBuffer)
            VertexBuffer->Release();
        if (IndexBuffer)
            IndexBuffer->Release();

        BufferIndexes.Add(bufferIndex);
    }

    for (uint32 buffer_index : BufferIndexes)
    {
        CachedBuffers[MaterialName].Remove(buffer_index);
    } 
}

void FRenderer::UpdateBoundingBoxBuffer(ID3D11Buffer* pBoundingBoxBuffer, const TArray<FBoundingBox>& BoundingBoxes, int numBoundingBoxes) const
{
    if (!pBoundingBoxBuffer) return;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    Graphics->DeviceContext->Map(pBoundingBoxBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    auto pData = reinterpret_cast<FBoundingBox*>(mappedResource.pData);
    for (int i = 0; i < BoundingBoxes.Num(); ++i)
    {
        pData[i] = BoundingBoxes[i];
    }
    Graphics->DeviceContext->Unmap(pBoundingBoxBuffer, 0);
}

void FRenderer::UpdateOBBBuffer(ID3D11Buffer* pBoundingBoxBuffer, const TArray<FOBB>& BoundingBoxes, int numBoundingBoxes) const
{
    if (!pBoundingBoxBuffer) return;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    Graphics->DeviceContext->Map(pBoundingBoxBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    auto pData = reinterpret_cast<FOBB*>(mappedResource.pData);
    for (int i = 0; i < BoundingBoxes.Num(); ++i)
    {
        pData[i] = BoundingBoxes[i];
    }
    Graphics->DeviceContext->Unmap(pBoundingBoxBuffer, 0);
}

void FRenderer::UpdateConesBuffer(ID3D11Buffer* pConeBuffer, const TArray<FCone>& Cones, int numCones) const
{
    if (!pConeBuffer) return;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    Graphics->DeviceContext->Map(pConeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    auto pData = reinterpret_cast<FCone*>(mappedResource.pData);
    for (int i = 0; i < Cones.Num(); ++i)
    {
        pData[i] = Cones[i];
    }
    Graphics->DeviceContext->Unmap(pConeBuffer, 0);
}

void FRenderer::UpdateGridConstantBuffer(const FGridParameters& gridParams) const
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = Graphics->DeviceContext->Map(GridConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(hr))
    {
        memcpy(mappedResource.pData, &gridParams, sizeof(FGridParameters));
        Graphics->DeviceContext->Unmap(GridConstantBuffer, 0);
    }
    else
    {
        UE_LOG(LogLevel::Warning, "gridParams ���� ����");
    }
}

void FRenderer::UpdateLinePrimitveCountBuffer(int numBoundingBoxes, int numCones) const
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = Graphics->DeviceContext->Map(LinePrimitiveBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    auto pData = static_cast<FPrimitiveCounts*>(mappedResource.pData);
    pData->BoundingBoxCount = numBoundingBoxes;
    pData->ConeCount = numCones;
    Graphics->DeviceContext->Unmap(LinePrimitiveBuffer, 0);
}

void FRenderer::RenderBatch(
    const FGridParameters& gridParam, ID3D11Buffer* pVertexBuffer, int boundingBoxCount, int coneCount, int coneSegmentCount, int obbCount
)
{
    UINT stride = sizeof(FSimpleVertex);
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);

    SetTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    UINT vertexCountPerInstance = 2;
    UINT instanceCount = gridParam.numGridLines + 3 + (boundingBoxCount * 12) + (coneCount * (2 * coneSegmentCount)) + (12 * obbCount);
    Graphics->DeviceContext->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);
    SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// UISOO TODO: 매 프레임 순회하며 넣어주고 있음. 변경사항 있는 거만 체크
void FRenderer::PrepareRender()
{
    // for (auto iter : TObjectRange<UStaticMeshComponent>())
    // {
    //     FMatrix Model = JungleMath::CreateModelMatrix(
    //         iter->GetWorldLocation(),
    //         iter->GetWorldRotation(),
    //         iter->GetWorldScale()
    //         );
    //
    //     FBoundingBox localBoundingBox = iter->AABB;
    //     if (Frustum.IsBoxVisible(FBoundingBox::TransformBy(localBoundingBox,iter->GetWorldLocation(), Model)))
    //     {
    //         StaticMeshObjs.Add(iter);
    //     }
    // }
    
    //for (const auto iter : TObjectRange<UGizmoBaseComponent>())
    //{
    //    GizmoObjs.Add(iter);
    //}

    //for (const auto iter : TObjectRange<UBillboardComponent>())
    //{
    //    BillboardObjs.Add(iter);
    //}

    //for (const auto iter : TObjectRange<ULightComponentBase>())
    //{
    //    LightObjs.Add(iter);
    //}
    
    //CreateBatchRenderCache();
}

void FRenderer::ClearRenderArr()
{
    StaticMeshObjs.Empty();
    GizmoObjs.Empty();
    BillboardObjs.Empty();
    LightObjs.Empty();
}

void FRenderer::Render(UWorld* World, const std::shared_ptr<FEditorViewportClient>& ActiveViewport)
{
    Graphics->DeviceContext->RSSetViewports(1, &ActiveViewport->GetD3DViewport());
    Graphics->ChangeRasterizer(ActiveViewport->GetViewMode());
    ChangeViewMode(ActiveViewport->GetViewMode());  // 완료 - Lit 없앰, Current 비교하여 ConstantBuffer Update X, 연산 짧음.
    //UpdateLightBuffer();
    uint32 startTime, endTime;

    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000 || bWasOcculusionQueried) 
    {
        //ResolveOcclusionQueries();
        bWasOcculusionQueried = false;
    }
    
    // UISOO TODO: 여기 Set LineShader
    UPrimitiveBatch::GetInstance().RenderBatchLine(ActiveViewport->GetViewMatrix(), ActiveViewport->GetProjectionMatrix());

    startTime = FPlatformTime::Cycles64();
    // UISOO TODO: 여기 Set StaticMeshShader
    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_Primitives)) {
        // RenderStaticMeshes(World, ActiveViewport);
        RenderBakedBuffer();
    }
    
    
    endTime = FPlatformTime::Cycles64();
    FWindowsPlatformTime::GElapsedMap["renderStaticMesh"] = FWindowsPlatformTime::ToMilliseconds(endTime - startTime);

    // UISOO TODO: Depth Stencil 바인딩 중
    //RenderGizmos(World, ActiveViewport);

    // UISOO TODO: 여기 Set TextureShader
    //if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_BillboardText))
    //    RenderBillboards(World, ActiveViewport);
    
    //RenderLight(World, ActiveViewport);

    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) 
    {
        //IssueOcclusionQueries(ActiveViewport);
        bWasOcculusionQueried = true;
    }

    
    
    ClearRenderArr();
}

void FRenderer::RenderStaticMeshes(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{    
    PrepareShader();
    // for (UStaticMeshComponent* StaticMeshComp : StaticMeshObjs)
    // {
    //     FMatrix Model = JungleMath::CreateModelMatrix(
    //         StaticMeshComp->GetWorldLocation(),
    //         StaticMeshComp->GetWorldRotation(),
    //         StaticMeshComp->GetWorldScale()
    //     );
    //     // 최종 MVP 행렬
    //     FMatrix MVP = Model * ActiveViewport->GetViewMatrix() * ActiveViewport->GetProjectionMatrix();
    //     // 노말 회전시 필요 행렬
    //     // FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
    //     // FVector4 UUIDColor = StaticMeshComp->EncodeUUID() / 255.0f;
    //
    //     // bool bIsSelected = World->GetSelectedActor() == StaticMeshComp->GetOwner();
    //     // UpdateConstant(MVP, NormalMatrix, UUIDColor, bIsSelected);
    //     UpdateConstant(MVP);
    //
    //     // if (USkySphereComponent* skysphere = Cast<USkySphereComponent>(StaticMeshComp))
    //     // {
    //     //     UpdateTextureConstant(skysphere->UOffset, skysphere->VOffset);
    //     // }
    //     // else
    //     // {
    //     //     UpdateTextureConstant(0, 0);
    //     // }
    //
    //     // if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_AABB))
    //     // {
    //     //     UPrimitiveBatch::GetInstance().RenderAABB(
    //     //         StaticMeshComp->GetBoundingBox(),
    //     //         StaticMeshComp->GetWorldLocation(),
    //     //         Model
    //     //     );
    //     // }
    //
    //     if (!StaticMeshComp->GetStaticMesh()) continue;
    //
    //     OBJ::FStaticMeshRenderData* renderData = StaticMeshComp->GetStaticMesh()->GetRenderData();
    //     if (renderData == nullptr) continue;
    //
    //     RenderPrimitive(renderData, StaticMeshComp->GetStaticMesh()->GetMaterials(), StaticMeshComp->GetOverrideMaterials(), StaticMeshComp->GetselectedSubMeshIndex());
    // } 

    // TODO: 일반 개별 렌더링 다시 사용할 경우 이거 없애주면 됨.
    if (MaterialConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        Graphics->DeviceContext->Map(MaterialConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        {
            FMaterialConstants* constants = static_cast<FMaterialConstants*>(ConstantBufferMSR.pData);
            constants->DiffuseColor = {0, 0, 0};
            // constants->TransparencyScalar = MaterialInfo.TransparencyScalar;
            // constants->AmbientColor = MaterialInfo.Ambient;
            // constants->DensityScalar = MaterialInfo.DensityScalar;
            // constants->SpecularColor = MaterialInfo.Specular;
            // constants->SpecularScalar = MaterialInfo.SpecularScalar;
            // constants->EmmisiveColor = MaterialInfo.Emissive;
        }
        Graphics->DeviceContext->Unmap(MaterialConstantBuffer, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
    
    for (const auto& [MaterialName, _] : BatchRenderTargets)
    {
        // OBJ::FStaticMeshRenderData* renderData = StaticMeshComp->GetStaticMesh()->GetRenderData();
        UMaterial* Material = FManagerOBJ::GetMaterial(MaterialName);

        // 1. Bind Texture - Clear
        const auto& MaterialInfo = Material->GetMaterialInfo();
        if (MaterialInfo.bHasTexture == true)
        {
            std::shared_ptr<FTexture> texture = FEngineLoop::resourceMgr.GetTexture(MaterialInfo.DiffuseTexturePath);

            SetPSTextureSRV(0, 1, texture->TextureSRV);
            SetPSSamplerState(0, 1, texture->SamplerState);
        }
        else
        {
            SetPSTextureSRV(0, 1, nullptr);
            SetPSSamplerState(0, 1, nullptr);
        }        

        // 3. Draw - Clear
        UINT offset = 0;
        UINT stride = sizeof(FVertexSimple);

        const auto Buffers = CachedBuffers[MaterialName];
        
        for (const auto& [BufferIndex, BufferInfo] : Buffers)
        {
            ID3D11Buffer* VertexBuffer = BufferInfo.Key;
            ID3D11Buffer* IndexBuffer = BufferInfo.Value.Value;
            uint32 IndexCount = BufferInfo.Value.Key;
            
            if (VertexBuffer == nullptr || IndexCount == 0 || IndexBuffer == nullptr)
                continue;
            
            DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_R32_UINT;
            Graphics->DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
            Graphics->DeviceContext->IASetIndexBuffer(IndexBuffer, IndexBufferFormat, 0);
            Graphics->DeviceContext->DrawIndexed(IndexCount, 0, 0);
        }
    }
}

void FRenderer::RenderGizmos(const UWorld* World, const std::shared_ptr<FEditorViewportClient>& ActiveViewport)
{
    if (!World->GetSelectedActor())
    {
        return;
    }

    #pragma region GizmoDepth
        ID3D11DepthStencilState* DepthStateDisable = Graphics->DepthStateDisable;
        Graphics->DeviceContext->OMSetDepthStencilState(DepthStateDisable, 0);
    #pragma endregion GizmoDepth

    //  fill solid,  Wirframe 에서도 제대로 렌더링되기 위함
    Graphics->DeviceContext->RSSetState(FEngineLoop::graphicDevice.RasterizerStateSOLID);
    
    for (auto GizmoComp : GizmoObjs)
    {
        if ((GizmoComp->GetGizmoType()==UGizmoBaseComponent::ArrowX ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::ArrowY ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::ArrowZ)
            && World->GetEditorPlayer()->GetControlMode() != CM_TRANSLATION)
            continue;
        else if ((GizmoComp->GetGizmoType()==UGizmoBaseComponent::ScaleX ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::ScaleY ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::ScaleZ)
            && World->GetEditorPlayer()->GetControlMode() != CM_SCALE)
            continue;
        else if ((GizmoComp->GetGizmoType()==UGizmoBaseComponent::CircleX ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::CircleY ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::CircleZ)
            && World->GetEditorPlayer()->GetControlMode() != CM_ROTATION)
            continue;
        FMatrix Model = JungleMath::CreateModelMatrix(GizmoComp->GetWorldLocation(),
            GizmoComp->GetWorldRotation(),
            GizmoComp->GetWorldScale()
        );
        // FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
        // FVector4 UUIDColor = GizmoComp->EncodeUUID() / 255.0f;

        FMatrix MVP = Model * ActiveViewport->GetViewMatrix() * ActiveViewport->GetProjectionMatrix();

        // if (GizmoComp == World->GetPickingGizmo())
        //     UpdateConstant(MVP, NormalMatrix, UUIDColor, true);
        // else
        //     UpdateConstant(MVP, NormalMatrix, UUIDColor, false);

        UpdateConstant(MVP);

        if (!GizmoComp->GetStaticMesh()) continue;

        OBJ::FStaticMeshRenderData* renderData = GizmoComp->GetStaticMesh()->GetRenderData();
        if (renderData == nullptr) continue;

        RenderPrimitive(renderData, GizmoComp->GetStaticMesh()->GetMaterials(), GizmoComp->GetOverrideMaterials());
    }

    Graphics->DeviceContext->RSSetState(Graphics->GetCurrentRasterizer());

#pragma region GizmoDepth
    ID3D11DepthStencilState* originalDepthState = Graphics->DepthStencilState;
    Graphics->DeviceContext->OMSetDepthStencilState(originalDepthState, 0);
#pragma endregion GizmoDepth
}

void FRenderer::RenderBillboards(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    if (BillboardObjs.Num() == 0)
        return;
    PrepareTextureShader();
    PrepareSubUVConstant();
    for (auto BillboardComp : BillboardObjs)
    {
        UpdateSubUVConstant(BillboardComp->finalIndexU, BillboardComp->finalIndexV);

        FMatrix Model = BillboardComp->CreateBillboardMatrix();

        // 최종 MVP 행렬
        FMatrix MVP = Model * ActiveViewport->GetViewMatrix() * ActiveViewport->GetProjectionMatrix();
        // FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
        // FVector4 UUIDColor = BillboardComp->EncodeUUID() / 255.0f;
        // if (BillboardComp == World->GetPickingGizmo())
        //     UpdateConstant(MVP, NormalMatrix, UUIDColor, true);
        // else
        //     UpdateConstant(MVP, NormalMatrix, UUIDColor, false);

        UpdateConstant(MVP);

        if (UParticleSubUVComp* SubUVParticle = Cast<UParticleSubUVComp>(BillboardComp))
        {
            RenderTexturePrimitive(
                SubUVParticle->vertexSubUVBuffer, SubUVParticle->numTextVertices,
                SubUVParticle->indexTextureBuffer, SubUVParticle->numIndices, SubUVParticle->Texture->TextureSRV, SubUVParticle->Texture->SamplerState
            );
        }
        else if (UText* Text = Cast<UText>(BillboardComp))
        {
            FEngineLoop::renderer.RenderTextPrimitive(
                Text->vertexTextBuffer, Text->numTextVertices,
                Text->Texture->TextureSRV, Text->Texture->SamplerState
            );
        }
        else
        {
            RenderTexturePrimitive(
                BillboardComp->vertexTextureBuffer, BillboardComp->numVertices,
                BillboardComp->indexTextureBuffer, BillboardComp->numIndices, BillboardComp->Texture->TextureSRV, BillboardComp->Texture->SamplerState
            );
        }
    }
    PrepareShader();
}

void FRenderer::UpdateBatchRenderTarget(const std::shared_ptr<FEditorViewportClient>& ActiveViewport)
{
    if (GEngineLoop.GetWorld() == nullptr)
        return;
    
    // LevelEditor->GetActiveViewportClient()

    //BatchRenderTargets.Empty();

    uint64 startTime, endTime;
    startTime = FPlatformTime::Cycles64();

    // Prepare
    const auto ocTree = GEngineLoop.GetWorld()->GetOcTree();
    ocTree.PrepareCull([&](const FOctreeElement<UStaticMeshComponent>& element) {
        UStaticMeshComponent* pStaticMeshComp = element.element;
        pStaticMeshComp->bIsVisible = false;
    });

    // Octree의 FrustumCull을 호출하여, 프러스텀 내에 있는 요소들에 대해 처리합니다.
    const FFrustum Frustum(ActiveViewport->GetViewMatrix(), ActiveViewport->GetProjectionMatrix());
    ocTree.FrustumCull(Frustum);

    // Occlusion
    FVector cameraPos = ActiveViewport->ViewTransformPerspective.GetLocation();
    ocTree.OcclusionCull(cameraPos);

    // Execute Callback
    ocTree.ExecuteCallbackForVisible([&](const FOctreeElement<UStaticMeshComponent>& element) {
        UStaticMeshComponent* pStaticMeshComp = element.element;
        pStaticMeshComp->bIsVisible = true;

        const FBoundingBox localAABB = pStaticMeshComp->AABB;
        const FMatrix Model = JungleMath::CreateModelMatrix(
           pStaticMeshComp->GetWorldLocation(),
           pStaticMeshComp->GetWorldRotation(),
           pStaticMeshComp->GetWorldScale()
       );
        
        const auto viewport = ActiveViewport->GetD3DViewport();
        FBoundingBox aabb = FBoundingBox::TransformBy(localAABB,pStaticMeshComp->GetWorldLocation(), Model);
        float screenCoverage = FBoundingBox::ComputeBoundingBoxScreenCoverage(aabb.min, aabb.max, ActiveViewport->GetViewMatrix(), ActiveViewport->GetProjectionMatrix(), viewport.Width, viewport.Height);

        if (screenCoverage > 1 / 16)
            pStaticMeshComp->SetLODLevel(0);
        else if (screenCoverage > 1 / 256)
            pStaticMeshComp->SetLODLevel(1);
    });

    endTime = FPlatformTime::Cycles64();
    FWindowsPlatformTime::GElapsedMap["Culling"] = FWindowsPlatformTime::ToMilliseconds(endTime - startTime);
}

void FRenderer::SetTopology(const D3D11_PRIMITIVE_TOPOLOGY InPrimitiveTopology)
{
    if (CurrentPrimitiveTopology == InPrimitiveTopology)
        return;
    
    CurrentPrimitiveTopology = InPrimitiveTopology;

    Graphics->DeviceContext->IASetPrimitiveTopology(CurrentPrimitiveTopology);
}

void FRenderer::SetPSTextureSRV(uint32 StartSlot, uint32 NumViews, ID3D11ShaderResourceView* InSRV)
{
    if (!CurrentTextureSRV.Contains(StartSlot))
    {
        CurrentTextureSRV.Add(StartSlot, {0, nullptr});
    }
    
    if (CurrentTextureSRV[StartSlot].Value == InSRV && CurrentTextureSRV[StartSlot].Key == NumViews)
    {
        return;
    }

    CurrentTextureSRV[StartSlot].Key = NumViews;
    CurrentTextureSRV[StartSlot].Value = InSRV;

    if (InSRV == nullptr)
    {
        ID3D11ShaderResourceView* nullSRV[1] = {nullptr};
        Graphics->DeviceContext->PSSetShaderResources(StartSlot, NumViews, nullSRV);
    }
    else
    {
        
        Graphics->DeviceContext->PSSetShaderResources(StartSlot, NumViews, &InSRV);
    }
}

void FRenderer::SetPSSamplerState(uint32 StartSlot, uint32 NumSamplers, ID3D11SamplerState* InSamplerState)
{
    if (!CurrentSamplerState.Contains(StartSlot))
    {
        CurrentSamplerState.Add(StartSlot, {0, nullptr});
    }
    
    if (CurrentSamplerState[StartSlot].Value == InSamplerState && CurrentSamplerState[StartSlot].Key == NumSamplers)
    {
        return;
    }

    CurrentSamplerState[StartSlot].Key = NumSamplers;
    CurrentSamplerState[StartSlot].Value = InSamplerState;

    if (InSamplerState == nullptr)
    {
        ID3D11SamplerState* nullSampler[1] = {nullptr};
        Graphics->DeviceContext->PSSetSamplers(StartSlot, NumSamplers, nullSampler);
    }
    else
    {
        Graphics->DeviceContext->PSSetSamplers(StartSlot, NumSamplers, &InSamplerState);
    }
}

void FRenderer::SetVertexShader(ID3D11VertexShader* InVertexShader)
{
    if (CurrentVertexShader == InVertexShader)
        return;
    
    CurrentVertexShader = InVertexShader;
    
    Graphics->DeviceContext->VSSetShader(InVertexShader, nullptr, 0);
}

void FRenderer::SetPixelShader(ID3D11PixelShader* InPixelShader)
{
    if (CurrentPixelShader == InPixelShader)
        return;
    
    CurrentPixelShader = InPixelShader;
    
    Graphics->DeviceContext->PSSetShader(InPixelShader, nullptr, 0);
}

void FRenderer::SetInputLayout(ID3D11InputLayout* InInputLayout)
{
    if (CurrentInputLayout == InInputLayout)
        return;

    CurrentInputLayout = InInputLayout;
    
    Graphics->DeviceContext->IASetInputLayout(InInputLayout);
}

void FRenderer::SetVSConstantBuffers(uint32 StartSlot, uint32 NumBuffers, ID3D11Buffer* InConstantBufferPtr)
{
    if (!CurrentVSConstantBuffers.Contains(StartSlot))
    {
        CurrentVSConstantBuffers.Add(StartSlot, {0, nullptr});
    }
    
    if (CurrentVSConstantBuffers[StartSlot].Value == InConstantBufferPtr && CurrentVSConstantBuffers[StartSlot].Key == NumBuffers)
    {
        return;
    }

    CurrentVSConstantBuffers[StartSlot].Key = NumBuffers;
    CurrentVSConstantBuffers[StartSlot].Value = InConstantBufferPtr;
    Graphics->DeviceContext->VSSetConstantBuffers(StartSlot, NumBuffers, &InConstantBufferPtr);
}

void FRenderer::SetPSConstantBuffers(uint32 StartSlot, uint32 NumBuffers, ID3D11Buffer* InConstantBufferPtr)
{
    if (!CurrentPSConstantBuffers.Contains(StartSlot))
    {
        CurrentPSConstantBuffers.Add(StartSlot, {0, nullptr});
    }
    
    if (CurrentPSConstantBuffers[StartSlot].Value == InConstantBufferPtr && CurrentPSConstantBuffers[StartSlot].Key == NumBuffers)
    {
        return;
    }

    CurrentPSConstantBuffers[StartSlot].Key = NumBuffers;
    CurrentPSConstantBuffers[StartSlot].Value = InConstantBufferPtr;
    Graphics->DeviceContext->PSSetConstantBuffers(StartSlot, NumBuffers, &InConstantBufferPtr);
}


void FRenderer::IssueOcclusionQueries(const std::shared_ptr<FEditorViewportClient>& ActiveViewport)
{
    OcclusionRenderer->IssueQueries(this, ActiveViewport);
}

void FRenderer::ResolveOcclusionQueries()
{
    OcclusionRenderer->ResolveQueries();
}

void FRenderer::RenderLight(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    for (auto Light : LightObjs)
    {
        FMatrix Model = JungleMath::CreateModelMatrix(Light->GetWorldLocation(), Light->GetWorldRotation(), {1, 1, 1});
        UPrimitiveBatch::GetInstance().AddCone(Light->GetWorldLocation(), Light->GetRadius(), 15, 140, Light->GetColor(), Model);
        UPrimitiveBatch::GetInstance().RenderOBB(Light->GetBoundingBox(), Light->GetWorldLocation(), Model);
    }
}

