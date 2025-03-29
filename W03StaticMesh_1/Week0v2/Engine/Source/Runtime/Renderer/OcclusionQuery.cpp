#include "OcclusionQuery.h"
#include "EngineLoop.h"
#include <d3d11.h>
FOcclusionQuery::FOcclusionQuery()
{
    D3D11_QUERY_DESC desc = {};
    desc.Query = D3D11_QUERY_OCCLUSION;
    desc.MiscFlags = 0;

    HRESULT hr = FEngineLoop::graphicDevice.Device->CreateQuery(&desc, &Query);
}

FOcclusionQuery::~FOcclusionQuery()
{
    if (Query)
    {
        Query->Release();
        Query = nullptr;
    }
}
