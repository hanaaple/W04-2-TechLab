#include "EngineLoop.h"
#include "ImGuiManager.h"
#include "World.h"
#include "Camera/CameraComponent.h"
#include "PropertyEditor/ViewportTypePanel.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/UnrealEd.h"
#include "UnrealClient.h"
#include "slate/Widgets/Layout/SSplitter.h"
#include "LevelEditor/SLevelEditor.h"
#include "UnrealEd/SceneMgr.h"
#include "Stats/ScopeCycleCounter.h"
#include "PropertyEditor/FPSEditorPanel.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
    {
        return true;
    }
    int zDelta = 0;
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            //UGraphicsDevice 객체의 OnResize 함수 호출
            if (FEngineLoop::graphicDevice.SwapChain)
            {
                FEngineLoop::graphicDevice.OnResize(hWnd);
            }
            for (int i = 0; i < 4; i++)
            {
                if (GEngineLoop.GetLevelEditor())
                {
                    if (GEngineLoop.GetLevelEditor()->GetViewports()[i])
                    {
                        GEngineLoop.GetLevelEditor()->GetViewports()[i]->ResizeViewport(FEngineLoop::graphicDevice.SwapchainDesc);
                    }
                }
            }
        }
     Console::GetInstance().OnResize(hWnd);
    // ControlPanel::GetInstance().OnResize(hWnd);
    // PropertyPanel::GetInstance().OnResize(hWnd);
    // Outliner::GetInstance().OnResize(hWnd);
    // ViewModeDropdown::GetInstance().OnResize(hWnd);
    // ShowFlags::GetInstance().OnResize(hWnd);
        if (GEngineLoop.GetUnrealEditor())
        {
            GEngineLoop.GetUnrealEditor()->OnResize(hWnd);
        }
        ViewportTypePanel::GetInstance().OnResize(hWnd);
        break;
    case WM_MOUSEWHEEL:
        if (ImGui::GetIO().WantCaptureMouse)
            return 0;
        zDelta = GET_WHEEL_DELTA_WPARAM(wParam); // 휠 회전 값 (+120 / -120)
        if (GEngineLoop.GetLevelEditor())
        {
            if (GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->IsPerspective())
            {
                if (GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetIsOnRBMouseClick())
                {
                    GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->SetCameraSpeedScalar(
                        static_cast<float>(GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetCameraSpeedScalar() + zDelta * 0.01)
                    );
                }
                else
                {
                    GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->CameraMoveForward(zDelta * 0.1f);
                }
            }
            else
            {
                FEditorViewportClient::SetOthoSize(-zDelta * 0.01f);
            }
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

FGraphicsDevice FEngineLoop::graphicDevice;
FRenderer FEngineLoop::renderer;
FResourceMgr FEngineLoop::resourceMgr;
uint32 FEngineLoop::TotalAllocationBytes = 0;
uint32 FEngineLoop::TotalAllocationCount = 0;

FEngineLoop::FEngineLoop()
    : hWnd(nullptr)
    , UIMgr(nullptr)
    , GWorld(nullptr)
    , LevelEditor(nullptr)
    , UnrealEditor(nullptr)
{
}

int32 FEngineLoop::PreInit()
{
    return 0;
}

int32 FEngineLoop::Init(HINSTANCE hInstance)
{
    if (FSceneMgr::LoadSceneData(TEXT("Default.scene")) == false)
        assert(false, TEXT("Can't Load Default Scene"));
    //if (FSceneMgr::LoadSceneData(TEXT("Test.scene")) == false)
    //    assert(false, TEXT("Can't Load Test Scene"));
    
    FWindowsPlatformTime::InitTiming();

    /* must be initialized before window. */
    UnrealEditor = new UnrealEd();
    UnrealEditor->Initialize();

    WindowInit(hInstance);
    graphicDevice.Initialize(hWnd);
    renderer.Initialize(&graphicDevice);

    UIMgr = new UImGuiManager;
    UIMgr->Initialize(hWnd, graphicDevice.Device, graphicDevice.DeviceContext);

    resourceMgr.Initialize(&renderer, &graphicDevice);
    LevelEditor = new SLevelEditor();
    LevelEditor->Initialize();

    GWorld = FObjectFactory::ConstructObject<UWorld>();
    GWorld->Initialize();

    return 0;
}


void FEngineLoop::Render()
{
    graphicDevice.Prepare();

    // 신경 안써도 됨.
    if (LevelEditor->IsMultiViewport())
    {
        std::shared_ptr<FEditorViewportClient> viewportClient = GetLevelEditor()->GetActiveViewportClient();
        for (int i = 0; i < 4; ++i)
        {
            LevelEditor->SetViewportClient(i);
            renderer.PrepareRender();
            renderer.Render(GetWorld(),LevelEditor->GetActiveViewportClient());
        }
        GetLevelEditor()->SetViewportClient(viewportClient);
    }
    else
    {
        renderer.PrepareRender();  // UISOO TODO: 바꿔야됨.
        renderer.Render(GetWorld(),LevelEditor->GetActiveViewportClient());
    }
}

void FEngineLoop::Tick()
{
    //LARGE_INTEGER frequency;
    //const double targetFrameTime = 1000.0 / targetFPS; // 한 프레임의 목표 시간 (밀리초 단위)

    //QueryPerformanceFrequency(&frequency);

    //LARGE_INTEGER startTime, endTime;
    double elapsedTime = 1.0;

    while (bIsExit == false)
    {
        //QueryPerformanceCounter(&startTime);
        TStatId dummy;
        FScopeCycleCounter counter(dummy);

        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg); // 키보드 입력 메시지를 문자메시지로 변경
            DispatchMessage(&msg);  // 메시지를 WndProc에 전달

            if (msg.message == WM_QUIT)
            {
                bIsExit = true;
                break;
            }
        }

        Input();

        uint64 startTime, endTime;

        startTime = FPlatformTime::Cycles64();
        GWorld->Tick(elapsedTime);
        endTime = FPlatformTime::Cycles64();
        elapsedTimes.worldTickDuration = FWindowsPlatformTime::ToMilliseconds(endTime - startTime);

        startTime = FPlatformTime::Cycles64();
        LevelEditor->Tick(elapsedTime);
        endTime = FPlatformTime::Cycles64();
        elapsedTimes.levelEditorDuration = FWindowsPlatformTime::ToMilliseconds(endTime - startTime);

        startTime = FPlatformTime::Cycles64();
        Render();
        endTime = FPlatformTime::Cycles64();
        elapsedTimes.renderDuration = FWindowsPlatformTime::ToMilliseconds(endTime - startTime);


        startTime = FPlatformTime::Cycles64();
        UIMgr->BeginFrame();
        endTime = FPlatformTime::Cycles64();
        elapsedTimes.UIBeginDuration = FWindowsPlatformTime::ToMilliseconds(endTime - startTime);

        startTime = FPlatformTime::Cycles64();
        UnrealEditor->Render();
        endTime = FPlatformTime::Cycles64();
        elapsedTimes.UEDuration = FWindowsPlatformTime::ToMilliseconds(endTime - startTime);

        startTime = FPlatformTime::Cycles64();
        Console::GetInstance().Draw();
        endTime = FPlatformTime::Cycles64();
        elapsedTimes.ConsoleDuration = FWindowsPlatformTime::ToMilliseconds(endTime - startTime);

        ImGui::Begin("stat");
        ImGui::Text("WorldTick:\t%fms", elapsedTimes.worldTickDuration);
        ImGui::Text("LevelEditor:\t%fms", elapsedTimes.levelEditorDuration);
        ImGui::Text("render:\t%fms", elapsedTimes.renderDuration);
        ImGui::Text("UIBegin:\t%fms", elapsedTimes.UIBeginDuration);
        ImGui::Text("UEditor:\t%fms", elapsedTimes.UEDuration);
        ImGui::Text("Console:\t%fms", elapsedTimes.ConsoleDuration);
        ImGui::Text("UIEnd:\t%fms", elapsedTimes.UIEndDuration);
        ImGui::Text("PendingDestory:\t%fms", elapsedTimes.pendingDestroyTime);
        ImGui::Text("SwapBuffer:\t%fms", elapsedTimes.swapBufferTime);
        ImGui::End();

        startTime = FPlatformTime::Cycles64();
        UIMgr->EndFrame();
        endTime = FPlatformTime::Cycles64();
        elapsedTimes.UIEndDuration = FWindowsPlatformTime::ToMilliseconds(endTime - startTime);

        // Pending 처리된 오브젝트 제거
        startTime = FPlatformTime::Cycles64();
        GUObjectArray.ProcessPendingDestroyObjects();
        endTime = FPlatformTime::Cycles64();
        elapsedTimes.pendingDestroyTime = FWindowsPlatformTime::ToMilliseconds(endTime - startTime);

        startTime = FPlatformTime::Cycles64();
        graphicDevice.SwapBuffer();
        endTime = FPlatformTime::Cycles64();
        elapsedTimes.swapBufferTime = FWindowsPlatformTime::ToMilliseconds(endTime - startTime);
        
        /*
        do
        {
            Sleep(0);
            QueryPerformanceCounter(&endTime);
            elapsedTime = (endTime.QuadPart - startTime.QuadPart) * 1000.0 / frequency.QuadPart;
        }
        while (elapsedTime < targetFrameTime);
        */
        uint64 CycleDiff = counter.Finish();;
        elapsedTime = FWindowsPlatformTime::ToMilliseconds(CycleDiff);
        FPSEditorPanel::elapsedTime = elapsedTime;
    }
}

float FEngineLoop::GetAspectRatio(IDXGISwapChain* swapChain) const
{
    DXGI_SWAP_CHAIN_DESC desc;
    swapChain->GetDesc(&desc);
    return static_cast<float>(desc.BufferDesc.Width) / static_cast<float>(desc.BufferDesc.Height);
}

void FEngineLoop::Input()
{
    //if (GetAsyncKeyState('M') & 0x8000)
    //{
    //    if (!bTestInput)
    //    {
    //        bTestInput = true;
    //        if (LevelEditor->IsMultiViewport())
    //        {
    //            LevelEditor->OffMultiViewport();
    //        }
    //        else
    //            LevelEditor->OnMultiViewport();
    //    }
    //}
    //else
    //{
    //    bTestInput = false;
    //}
}

void FEngineLoop::Exit()
{
    LevelEditor->Release();
    GWorld->Release();
    delete GWorld;
    UIMgr->Shutdown();
    delete UIMgr;
    resourceMgr.Release(&renderer);
    renderer.Release();
    graphicDevice.Release();
}


void FEngineLoop::WindowInit(HINSTANCE hInstance)
{
    WCHAR WindowClass[] = L"JungleWindowClass";

    WCHAR Title[] = L"Game Tech Lab";

    WNDCLASSW wndclass = {0};
    wndclass.lpfnWndProc = WndProc;
    wndclass.hInstance = hInstance;
    wndclass.lpszClassName = WindowClass;

    RegisterClassW(&wndclass);

    hWnd = CreateWindowExW(
        0, WindowClass, Title, WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1000, 1000,
        nullptr, nullptr, hInstance, nullptr
    );
}
