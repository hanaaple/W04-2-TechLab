#include "FRenderingThread.h"
#include "Renderer/CommandList.h"

void RenderingThread::ThreadLoop()
{
        if (bUseDeferredContext)
        {
            // Deferred Context 초기화 코드 구현
        }
        else
        {
            // Immediate Context 초기화 코드 구현
        }
        // DirectX11 장치, 컨텍스트, 스왑체인 등의 초기화 코드가 이곳에 들어갑니다.

        while (true)
        {
            // LockFreeQueue 기반이므로, 큐가 비었으면 짧게 Sleep하여 CPU 부하를 낮춥니다.
            while (CommandBuffer.IsEmpty() && !bExitFlag)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            // 종료 플래그가 설정되고 큐가 비었으면 루프 종료
            if (bExitFlag && CommandBuffer.IsEmpty())
                break;

            // 큐에 있는 모든 명령 실행
            CommandBuffer.ProcessCommands();

            if (bUseDeferredContext)
            {
                // Deferred Context 모드: 명령 기록 후 Command List 제출
                // 예제에서는 FCommandList 객체를 생성해 명령을 기록한 후 제출하는 식으로 처리
                FCommandList cmdList(m_device.Get(), m_rtv.Get());
                float clearColor[4] = { 0.1f, 0.2f, 0.3f, 1.0f };
                cmdList.ClearRenderTarget(clearColor);
                // 추가 명령 기록 가능...
                cmdList.Finish();
                // Immediate Context에 명령 리스트 제출 (실제 엔진에서는 Immediate Context가 필요)
                // g_pImmediateContext->ExecuteCommandList(cmdList.GetCommandList(), FALSE);
            }
            else
            {
                // Immediate Context 모드: 이미 즉시 실행할 명령들이 처리되었다고 가정
            }

            // 프레임 Present (예제에서는 출력)
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    
        // DirectX11 리소스 해제 코드
    }