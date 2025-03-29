#pragma once
#include <d3d11.h>
#include <wrl/client.h>

//------------------------------------------------------------------------------
// FCommandList: Deferred Context로 렌더 명령을 기록하고, Command List로 완성하는 클래스
//------------------------------------------------------------------------------
class FCommandList
{
public:
    FCommandList(ID3D11Device* device, ID3D11RenderTargetView* rtv)
        : Device(device), RenderTargetView(rtv)
    {
        // Deferred Context 생성 (옵션 플래그 0)
        Device->CreateDeferredContext(0, &DeferredContext);
    }

    // 예제: 렌더 타겟 클리어 명령 기록
    void ClearRenderTarget(const float clearColor[4]) const
    {
        DeferredContext->ClearRenderTargetView(RenderTargetView.Get(), clearColor);
    }

    // 기타 명령들도 여기에 기록 가능 (예: Draw, SetState 등)

    // 기록 완료 후 명령 리스트 생성
    void Finish()
    {
        DeferredContext->FinishCommandList(FALSE, &CommandList);
    }

    // 생성된 Command List 반환
    ID3D11CommandList* GetCommandList() const
    {
        return CommandList.Get();
    }

private:
    Microsoft::WRL::ComPtr<ID3D11Device>            Device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>     DeferredContext;
    Microsoft::WRL::ComPtr<ID3D11CommandList>       CommandList;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  RenderTargetView;
};