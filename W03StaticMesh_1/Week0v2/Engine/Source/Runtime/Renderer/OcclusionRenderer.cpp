#include "Launch/Define.h"
#include "OcclusionRenderer.h"
#include "Engine/ResourceMgr.h"
#include "OcclusionQuery.h"
#include "Components/StaticMeshComponent.h"
#include "Math/JungleMath.h"
#include <d3d11.h>
#include <d3dcompiler.h>

void FOcclusionRenderer::Initialize(FGraphicsDevice* graphics)
{
    Graphics = graphics;
    CreateShader();
    CreateConstantBuffer();
    CreateVertexBuffer();
}

void FOcclusionRenderer::Prepare()
{
    ID3D11DeviceContext* context = Graphics->DeviceContext;

    context->VSSetShader(OcclusionVertexShader, nullptr, 0);
    context->PSSetShader(nullptr, nullptr, 0);
    context->IASetInputLayout(nullptr);
    context->VSSetConstantBuffers(0, 1, &BoxConstantBuffer);
}

void FOcclusionRenderer::IssueQueries(TArray<UStaticMeshComponent*>& StaticMeshObjs, const FMatrix& VP)
{
    Prepare();
    for (UStaticMeshComponent* StaticMeshComp : StaticMeshObjs)
    {
        FMatrix Model = JungleMath::CreateModelMatrix(
            StaticMeshComp->GetWorldLocation(),
            StaticMeshComp->GetWorldRotation(),
            StaticMeshComp->GetWorldScale()
        );

        FMatrix MVP = Model * VP;
        const FBoundingBox& LocalBounds = StaticMeshComp->GetBoundingBox();

        IssueQuery(LocalBounds, MVP,StaticMeshComp->query);

    }
}

void FOcclusionRenderer::IssueQuery(const FBoundingBox& box, const FMatrix& MVP, const FOcclusionQuery& query)
{
    if (query.Get() == nullptr) return;
    Graphics->DeviceContext->Begin(query.Get());
    UpdateBoxConstantBuffer(box, MVP);
    Rendering(box);
    Graphics->DeviceContext->End(query.Get());
}

void FOcclusionRenderer::ResolveQueries(const TArray<UStaticMeshComponent*>& StaticMeshObjs)
{
    ID3D11DeviceContext* context = Graphics->DeviceContext;

    for (UStaticMeshComponent* StaticMeshComp : StaticMeshObjs)
    {
        ID3D11Query* query = StaticMeshComp->query.Get();

        if (query == nullptr)
        {
            StaticMeshComp->bIsVisible = true; // 쿼리가 없으면 기본 true 처리
            continue;
        }

        UINT64 pixelCount = 0;
        HRESULT hr = context->GetData(query, &pixelCount, sizeof(UINT64), 0);

        if (hr == S_OK)
        {
            StaticMeshComp->bIsVisible = (pixelCount > 0);
        }
        else
        {
            // 결과가 아직 GPU에서 안 나왔거나 실패했으면 conservative fallback
            StaticMeshComp->bIsVisible = true;
        }
    }
}

void FOcclusionRenderer::Release()
{
    if (OcclusionVertexShader) OcclusionVertexShader->Release();
    if (BoxConstantBuffer) BoxConstantBuffer->Release();
    if (DummyVertexBuffer) DummyVertexBuffer->Release();
}

void FOcclusionRenderer::UpdateBoxConstantBuffer(const FBoundingBox& box,const FMatrix& mvp)
{
    FBoxConstantBuffer cb;
    cb.MVP = mvp;
    cb.Min = box.min;
    cb.Max = box.max;

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    Graphics->DeviceContext->Map(BoxConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, &cb, sizeof(cb));
    Graphics->DeviceContext->Unmap(BoxConstantBuffer,0);

}

void FOcclusionRenderer::Rendering(const FBoundingBox& box)
{
    ID3D11DeviceContext* context = Graphics->DeviceContext;

    // Dummy Vertex Buffer (float dummy = 0.0f)
    UINT stride = sizeof(float);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &DummyVertexBuffer, &stride, &offset);

    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    // Depth-only 상태 설정 필요 (Raster/Depth/BlendState 등)

    // 드로우: vertex 1개 * 인스턴스 8개
    context->DrawInstanced(1, 8, 0, 0);
}

void FOcclusionRenderer::CreateShader()
{
    ID3DBlob* vertexBlob;

    HRESULT hr;
    hr = D3DCompileFromFile(L"Shaders/OcclusionShader.hlsl", nullptr, nullptr,
        "main", "vs_5_0", 0, 0, &vertexBlob, nullptr);
    Graphics->Device->CreateVertexShader(vertexBlob->GetBufferPointer(),
        vertexBlob->GetBufferSize(), nullptr, &OcclusionVertexShader);

    vertexBlob->Release();
}

void FOcclusionRenderer::CreateConstantBuffer()
{
    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = sizeof(FBoxConstantBuffer);
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    Graphics->Device->CreateBuffer(&desc, nullptr, &BoxConstantBuffer);
}

void FOcclusionRenderer::CreateVertexBuffer()
{
    float dummy = 0.0f;

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vbDesc.ByteWidth = sizeof(float);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = {};
    vbData.pSysMem = &dummy;

    Graphics->Device->CreateBuffer(&vbDesc, &vbData, &DummyVertexBuffer);

}
