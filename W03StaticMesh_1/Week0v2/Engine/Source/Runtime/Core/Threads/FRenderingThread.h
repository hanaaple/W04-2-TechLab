#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

#include "Container/LockFreeQueue.h"

// FRenderCommandBuffer: 게임 쓰레드에서 렌더링 명령을 넣고 렌더링 쓰레드에서 처리하는 큐
class FRenderCommandBuffer
{
public:
    using RenderCommandFunc = std::function<void()>;

    FRenderCommandBuffer() = default;
    ~FRenderCommandBuffer() = default;

    // 게임 쓰레드: 렌더 명령(람다 함수)을 큐에 추가합니다.
    void EnqueueCommand(RenderCommandFunc Cmd)
    {
        commandQueue.Enqueue(std::move(Cmd));
    }

    // 렌더링 쓰레드: 큐에서 렌더 명령을 하나씩 제거하여 실행합니다.
    void ProcessCommands()
    {
        RenderCommandFunc Cmd;
        // TLockFreeQueue의 Dequeue()는 std::optional<T>를 반환합니다.
        while (true)
        {
            auto optCmd = commandQueue.Dequeue();
            if (!optCmd.has_value())
                break;
            Cmd = std::move(optCmd.value());
            if (Cmd)
            {
                Cmd();
            }
        }
    }

    // 큐가 비어있는지 여부를 확인합니다.
    bool IsEmpty() const
    {
        return commandQueue.IsEmpty();
    }

private:
    TLockFreeQueue<RenderCommandFunc> commandQueue;
};


// RenderingThread: DirectX11 초기화/해제 및 렌더링 루프를 관리하는 클래스.
// 내부적으로 FRenderCommandBufferLF를 사용하여 게임 쓰레드에서 전달된 렌더 명령을 처리합니다.
class RenderingThread
{
public:
    RenderingThread(bool useDeferredContext, ID3D11Device* device, ID3D11RenderTargetView* rtv)
        : bExitFlag(false)
        , bUseDeferredContext(useDeferredContext)
        , m_device(device)
        , m_rtv(rtv)
    {}

    ~RenderingThread()
    {
        Stop();
    }

    // 렌더링 쓰레드를 시작합니다.
    void Start()
    {
        bExitFlag = false;
        RenderThreadHandle = std::thread(&RenderingThread::ThreadLoop, this);
    }

    // 렌더링 쓰레드를 종료합니다.
    void Stop()
    {
        bExitFlag = true;
        // LockFreeQueue는 락 없이 동작하므로, 종료 플래그가 true가 되고 명령이 없을 때 루프를 종료하도록 합니다.
        if (RenderThreadHandle.joinable())
        {
            RenderThreadHandle.join();
        }
    }

    // 게임 쓰레드에서 렌더 명령을 추가합니다.
    void EnqueueCommand(FRenderCommandBuffer::RenderCommandFunc Cmd)
    {
        CommandBuffer.EnqueueCommand(std::move(Cmd));
    }

private:
    // 렌더링 쓰레드 루프
    void ThreadLoop();

    std::thread RenderThreadHandle;
    std::atomic<bool> bExitFlag;
    FRenderCommandBuffer CommandBuffer;
    bool bUseDeferredContext;
    Microsoft::WRL::ComPtr<ID3D11Device> m_device;              // DirectX11 장치 (초기화 시 전달)
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;         // 렌더 타겟 뷰 (초기화 시 전달)
};

