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

    D3DCompileFromFile(L"Shaders/StaticMeshVertexShader.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &VertexShaderCSO, nullptr);
    Graphics->Device->CreateVertexShader(VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &VertexShader);

    D3DCompileFromFile(L"Shaders/StaticMeshPixelShader.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &PixelShaderCSO, nullptr);
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

    D3D11_SUBRESOURCE_DATA vertexbufferSRD;
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
            CachedBuffers.Add(MaterialName, TArray<TPair<ID3D11Buffer*, TPair<uint32, ID3D11Buffer*>>>());
        
        CachedBuffers[MaterialName].Empty();

        int MeshIndex = 0;
        while (BatchRenderTargetContext.StaticMeshes.Num() > MeshIndex)
        {
            uint32 VertexOffset = 0;
            TArray<FVertexSimple> VertexData;
            TArray<uint32> IndexData;

            for (; MeshIndex < BatchRenderTargetContext.StaticMeshes.Num(); MeshIndex++)
            {
                UStaticMeshComponent* StaticMeshComponent = BatchRenderTargetContext.StaticMeshes[MeshIndex].Value;
                const uint32 SubMeshIndex = BatchRenderTargetContext.StaticMeshes[MeshIndex].Key;
                const OBJ::FStaticMeshRenderData* renderData = StaticMeshComponent->GetStaticMesh()->GetRenderData();
                const uint32 IndexBufferStartIndex = renderData->MaterialSubsets[SubMeshIndex].IndexStart;
                const uint32 IndexCount = renderData->MaterialSubsets[SubMeshIndex].IndexCount;

                TArray<FVertexSimple> Vertices = StaticMeshComponent->Vertices;

                // UISOO TODO: 사용하지 않는 Vertex 없애고 Indexing 당기기
                VertexData.Append(Vertices);

                const TArray<uint32>& Indices = renderData->Indices;
                for (int i = IndexBufferStartIndex; i < IndexBufferStartIndex + IndexCount; i++)
                {
                    IndexData.Add(Indices[i] + VertexOffset);                    
                }
                
                VertexOffset += Vertices.Num();

                static const uint32 MaxVertexBufferSize = 16 * 1024 * 1024;
                //static const uint32 MaxIndexBufferSize = 2 * 1024 * 1024;
                //uint32 IndexByte = IndexData.Num() * sizeof(uint32);
                uint32 VertexByte = VertexOffset * sizeof(FVertexSimple);
                if (VertexByte > MaxVertexBufferSize
                    //|| IndexByte > MaxIndexBufferSize
                    )
                    break;
            }

            uint32 VertexDataSize = sizeof(FVertexSimple) * VertexData.Num();
            ID3D11Buffer* VertexBuffer = CreateVertexBuffer(VertexData.GetData(), VertexDataSize);
            //ID3D11Buffer* VertexBuffer = UpdateOrCreateVertexBuffer(MaterialName, MeshIndex, VertexData.GetData(), VertexDataSize);
        
            uint32 IndexDataSize = sizeof(uint32) * IndexData.Num();
            ID3D11Buffer* IndexBuffer = CreateIndexBuffer(IndexData.GetData(), IndexDataSize);
            //ID3D11Buffer* IndexBuffer = UpdateOrCreateIndexBuffer(MaterialName, MeshIndex, IndexData.GetData(), IndexDataSize);
        
            CachedBuffers[MaterialName].Add({VertexBuffer, {IndexDataSize, IndexBuffer}});

            MeshIndex++;
        }
    }
}

ID3D11Buffer* FRenderer::UpdateOrCreateVertexBuffer(const FString& MaterialName, uint32 MeshIndex, FVertexSimple* Data, uint32 VertexDataSize)
{
    return nullptr;
}

ID3D11Buffer* FRenderer::UpdateOrCreateIndexBuffer(const FString& MaterialName, uint32 MeshIndex, void* Data, uint32 IndexDataSize)
{
    return nullptr;
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
    
    for (const auto iter : TObjectRange<UGizmoBaseComponent>())
    {
        GizmoObjs.Add(iter);
    }

    //for (const auto iter : TObjectRange<UBillboardComponent>())
    //{
    //    BillboardObjs.Add(iter);
    //}

    //for (const auto iter : TObjectRange<ULightComponentBase>())
    //{
    //    LightObjs.Add(iter);
    //}
    
    CreateBatchRenderCache();
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

    uint64 startTime, endTime;

    startTime = FPlatformTime::Cycles64();
    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) 
    {
        //ResolveOcclusionQueries();
    }
    endTime = FPlatformTime::Cycles64();
    FWindowsPlatformTime::GElapsedMap["resolveOcclusion"] = FWindowsPlatformTime::ToMilliseconds(endTime - startTime);
    
    // UISOO TODO: 여기 Set LineShader
    UPrimitiveBatch::GetInstance().RenderBatchLine(ActiveViewport->GetViewMatrix(), ActiveViewport->GetProjectionMatrix());
    
    startTime = FPlatformTime::Cycles64();
    // UISOO TODO: 여기 Set StaticMeshShader
    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_Primitives))
        RenderStaticMeshes(World, ActiveViewport);
    endTime = FPlatformTime::Cycles64();
    FWindowsPlatformTime::GElapsedMap["renderStaticMesh"] = FWindowsPlatformTime::ToMilliseconds(endTime - startTime);

    // UISOO TODO: Depth Stencil 바인딩 중
    RenderGizmos(World, ActiveViewport);

    // UISOO TODO: 여기 Set TextureShader
    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_BillboardText))
        RenderBillboards(World, ActiveViewport);
    
    //RenderLight(World, ActiveViewport);

    startTime = FPlatformTime::Cycles64();
    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) 
    {
        //IssueOcclusionQueries(ActiveViewport);
    }
    endTime = FPlatformTime::Cycles64();
    FWindowsPlatformTime::GElapsedMap["issueOcclusion"] = FWindowsPlatformTime::ToMilliseconds(endTime - startTime);
    
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

        const auto Buffers = GetCachedBuffers(MaterialName);

        for (const auto& [VertexBuffer, IndexBufferInfo] : Buffers)
        {
            if (VertexBuffer == nullptr || IndexBufferInfo.Key == 0 || IndexBufferInfo.Value == nullptr)
                continue;
            
            DXGI_FORMAT IndexBufferFormat = DXGI_FORMAT_R32_UINT;
            uint32 IndexCount = IndexBufferInfo.Key;
            ID3D11Buffer* IndexBuffer = IndexBufferInfo.Value;
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

void FRenderer::UpdateBatchRenderTarget(std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    if (GEngineLoop.GetWorld() == nullptr)
        return;
    
    // LevelEditor->GetActiveViewportClient()

    BatchRenderTargets.Empty();
    
    const FFrustum Frustum(ActiveViewport->GetViewMatrix(), ActiveViewport->GetProjectionMatrix());

    // Octree의 FrustumCull을 호출하여, 프러스텀 내에 있는 요소들에 대해 처리합니다.
    const auto ocTree =  GEngineLoop.GetWorld()->GetOcTree();
    //ocTree.FrustumCull(Frustum, [&](const FOctreeElement<UStaticMeshComponent>& element)
    //{
    //    UStaticMeshComponent* pStaticMeshComp = element.element;
    //    if (!pStaticMeshComp->bIsVisible)
    //        return;
    //    if (Cast<UGizmoBaseComponent>(pStaticMeshComp))
    //        return;
    //    // StaticMeshObjs.Add(pStaticMeshComp);
    //            
    //    for (uint32 i = 0; i < pStaticMeshComp->GetNumMaterials(); i++)
    //    {
    //        auto Material = pStaticMeshComp->GetMaterial(i);
    //        auto MTLName = Material->GetMaterialInfo().MTLName;
    //        if (!BatchRenderTargets.Contains(MTLName))
    //        {
    //            BatchRenderTargets.Add(MTLName, BatchRenderTargetContext());
    //            BatchRenderTargets[MTLName].bIsDirty = true;
    //        }
    //        if (BatchRenderTargets[MTLName].bIsDirty)
    //        {
    //            BatchRenderTargets[MTLName].StaticMeshes.Add({ i, pStaticMeshComp });
    //        }
    //        
    //        // Material의 변경, Transform의 변경, Culling에 의한 삭제에 따라 Targets 초기화 (BatchRenderTargets[MTLName].Empty();
    //    }
    //});

    FVector cameraPos = ActiveViewport->ViewTransformPerspective.GetLocation();
    ocTree.OcclusionCull(cameraPos, [&](const FOctreeElement<UStaticMeshComponent>& element) {
        UStaticMeshComponent* pStaticMeshComp = element.element;
        if ( !pStaticMeshComp->bIsVisible )
            return;
        if ( Cast<UGizmoBaseComponent>(pStaticMeshComp) )
            return;

        for ( uint32 i = 0; i < pStaticMeshComp->GetNumMaterials(); i++ ) {
            auto Material = pStaticMeshComp->GetMaterial(i);
            auto MTLName = Material->GetMaterialInfo().MTLName;
            if ( !BatchRenderTargets.Contains(MTLName) ) {
                BatchRenderTargets.Add(MTLName, BatchRenderTargetContext());
                BatchRenderTargets[MTLName].bIsDirty = true;
            }
            if ( BatchRenderTargets[MTLName].bIsDirty ) {
                BatchRenderTargets[MTLName].StaticMeshes.Add({ i, pStaticMeshComp });
            }
        }
    });
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

TArray<TPair<ID3D11Buffer*, TPair<uint32, ID3D11Buffer*>>> FRenderer::GetCachedBuffers(const FString& InMaterialName)
{
    return CachedBuffers[InMaterialName];
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
