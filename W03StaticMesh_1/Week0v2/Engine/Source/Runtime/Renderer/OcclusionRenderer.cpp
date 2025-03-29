#include "OcclusionRenderer.h"
#include "Engine/ResourceMgr.h"
#include <d3d11.h>
#include <d3dcompiler.h>

void FOcclusionRenderer::Initialize(FGraphicsDevice* graphics)
{
    Graphics = graphics;
    CreateShader();
    CreateConstantBuffer();
}

void FOcclusionRenderer::Prepare()
{
    ID3D11DeviceContext* context = Graphics->DeviceContext;

    context->VSSetShader(OcclusionVertexShader, nullptr, 0);
    context->PSSetShader(nullptr, nullptr, 0);
    context->IASetInputLayout(InputLayout);
}

void FOcclusionRenderer::IssueQueries(const FBoundingBox& box, const FMatrix& MVP)
{

}

void FOcclusionRenderer::ResolveQueries()
{
}

void FOcclusionRenderer::Release()
{
    if (OcclusionVertexShader) OcclusionVertexShader->Release();
    if (InputLayout) InputLayout->Release();
    if (MVPConstantBuffer) MVPConstantBuffer->Release();
}

void FOcclusionRenderer::UpdateMVPConstantBuffer(const FMatrix& mvp)
{
    MVPConstant cb = {};
    cb.MVP = mvp;
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    Graphics->DeviceContext->Map(MVPConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    memcpy(mapped.pData, &cb, sizeof(cb));
    Graphics->DeviceContext->Unmap(MVPConstantBuffer,0);

}



void FOcclusionRenderer::CreateShader()
{
    ID3DBlob* vertexBlob;

    HRESULT hr;
    hr = D3DCompileFromFile(L"Shaders/OcclusionShader.hlsl", nullptr, nullptr,
        "main", "vs_5_0", 0, 0, &vertexBlob, nullptr);
    Graphics->Device->CreateVertexShader(vertexBlob->GetBufferPointer(),
        vertexBlob->GetBufferSize(), nullptr, &OcclusionVertexShader);

    D3D11_INPUT_ELEMENT_DESC layout[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
      D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    Graphics->Device->CreateInputLayout(layout, 1,
        vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), &InputLayout);

    vertexBlob->Release();
}

void FOcclusionRenderer::CreateConstantBuffer()
{
    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.ByteWidth = sizeof(MVPConstant);
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    Graphics->Device->CreateBuffer(&desc, nullptr, &MVPConstantBuffer);
}
