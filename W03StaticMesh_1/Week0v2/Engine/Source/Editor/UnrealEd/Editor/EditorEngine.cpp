#include "EditorEngine.h"
#include "ImGuiManager.h"
#include "World.h"
#include "PropertyEditor/ViewportTypePanel.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/UnrealEd.h"
#include "UnrealClient.h"
#include "slate/Widgets/Layout/SSplitter.h"
#include "LevelEditor/SLevelEditor.h"
#include "Renderer/Renderer.h"
#include "UObject/UObjectGlobals.h"


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
            if (FEditorEngine::graphicDevice.SwapChain)
            {
                FEditorEngine::graphicDevice.OnResize(hWnd);
            }
            for (int i = 0; i < 4; i++)
            {
                if (GEngineLoop.GetLevelEditor())
                {
                    if (GEngineLoop.GetLevelEditor()->GetViewports()[i])
                    {
                        GEngineLoop.GetLevelEditor()->GetViewports()[i]->ResizeViewport(FEditorEngine::graphicDevice.SwapchainDesc);
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
            FEditorViewportClient* viewportClient = dynamic_cast<FEditorViewportClient*>(GEngineLoop.GetLevelEditor()->GetActiveViewportClient().get());
            if (viewportClient)
            {
                if (viewportClient->IsPerspective())
                {
                    if (viewportClient->GetIsOnRBMouseClick())
                    {
                        viewportClient->SetCameraSpeedScalar(
                            static_cast<float>(GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetCameraSpeedScalar() + zDelta * 0.01)
                        );
                    }
                    else
                    {
                        viewportClient->CameraMoveForward(zDelta * 0.1f);
                    }
                }
                else
                {
                    FEditorViewportClient::SetOthoSize(-zDelta * 0.01f);
                }
            } 
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

FGraphicsDevice FEditorEngine::graphicDevice;
FRenderer FEditorEngine::renderer;
FResourceMgr FEditorEngine::resourceMgr;
uint32 FEditorEngine::TotalAllocationBytes = 0;
uint32 FEditorEngine::TotalAllocationCount = 0;

FEditorEngine::FEditorEngine()
    : hWnd(nullptr)
    , UIMgr(nullptr)
    , GWorld(nullptr)
    , LevelEditor(nullptr)
    , UnrealEditor(nullptr)
{
}

int32 FEditorEngine::PreInit()
{
    return 0;
}

int32 FEditorEngine::Init(HINSTANCE hInstance)
{
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
    GWorld->Initialize(EWorldType::Editor);

    return 0;
}


void FEditorEngine::Render()
{
    graphicDevice.Prepare();
    if (GWorld->GetWorldType() == EWorldType::Editor)
    {
        
        if (LevelEditor->IsMultiViewport())
        {
            std::shared_ptr<FEditorViewportClient> viewportClient = std::dynamic_pointer_cast<FEditorViewportClient>(GetLevelEditor()->GetActiveViewportClient());
            for (int i = 0; i < 4; ++i)
            {
                LevelEditor->SetViewportClient(i);
                // graphicDevice.DeviceContext->RSSetViewports(1, &LevelEditor->GetViewports()[i]->GetD3DViewport());
                // graphicDevice.ChangeRasterizer(LevelEditor->GetActiveViewportClient()->GetViewMode());
                // renderer.ChangeViewMode(LevelEditor->GetActiveViewportClient()->GetViewMode());
                // renderer.PrepareShader();
                // renderer.UpdateLightBuffer();
                // RenderWorld();
                renderer.PrepareRender();
                renderer.Render(GetWorld(),LevelEditor->GetActiveViewportClient());
            }
            GetLevelEditor()->SetViewportClient(viewportClient);
        }
        else
        {
            // graphicDevice.DeviceContext->RSSetViewports(1, &LevelEditor->GetActiveViewportClient()->GetD3DViewport());
            // graphicDevice.ChangeRasterizer(LevelEditor->GetActiveViewportClient()->GetViewMode());
            // renderer.ChangeViewMode(LevelEditor->GetActiveViewportClient()->GetViewMode());
            // renderer.PrepareShader();
            // renderer.UpdateLightBuffer();
            // RenderWorld();
            renderer.PrepareRender();
            renderer.Render(GetWorld(),LevelEditor->GetActiveViewportClient());
        }
        
    } else if (GWorld->GetWorldType() == EWorldType::PIE)
    {
        renderer.PrepareRender();
        renderer.Render(GetWorld(), LevelEditor->GetActiveViewportClient());
    }
}

void FEditorEngine::Tick()
{
    LARGE_INTEGER frequency;
    const double targetFrameTime = 1000.0 / targetFPS; // 한 프레임의 목표 시간 (밀리초 단위)

    QueryPerformanceFrequency(&frequency);

    LARGE_INTEGER startTime, endTime;
    double elapsedTime = 1.0;

    while (bIsExit == false)
    {
        QueryPerformanceCounter(&startTime);

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
        GWorld->Tick(elapsedTime);
        LevelEditor->Tick(elapsedTime);
        Render();
        UIMgr->BeginFrame();
        UnrealEditor->Render();

        Console::GetInstance().Draw();

        UIMgr->EndFrame();

        // Pending 처리된 오브젝트 제거
        GUObjectArray.ProcessPendingDestroyObjects();

        graphicDevice.SwapBuffer();
        do
        {
            Sleep(0);
            QueryPerformanceCounter(&endTime);
            elapsedTime = (endTime.QuadPart - startTime.QuadPart) * 1000.0 / frequency.QuadPart;
        }
        while (elapsedTime < targetFrameTime);
    }
}

float FEditorEngine::GetAspectRatio(IDXGISwapChain* swapChain) const
{
    DXGI_SWAP_CHAIN_DESC desc;
    swapChain->GetDesc(&desc);
    return static_cast<float>(desc.BufferDesc.Width) / static_cast<float>(desc.BufferDesc.Height);
}

void FEditorEngine::Input()
{
    if (GetAsyncKeyState('M') & 0x8000)
    {
        if (!bTestInput)
        {
            bTestInput = true;
            if (LevelEditor->IsMultiViewport())
            {
                LevelEditor->OffMultiViewport();
            }
            else
                LevelEditor->OnMultiViewport();
        }
    }
    else
    {
        bTestInput = false;
    }
}

void FEditorEngine::Exit()
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


void FEditorEngine::WindowInit(HINSTANCE hInstance)
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

void FEditorEngine::StartPIE()
{
    OriginWorld = GWorld;
    GWorld = Cast<UWorld>(GWorld->Duplicate());
    GWorld->WorldType = EWorldType::PIE;
    ULevel* TargetLevel = GEngineLoop.GetWorld()->GetLevel();
    TargetLevel->LevelState = ELevelState::Play;
}

void FEditorEngine::EndPIE()
{
    FPlatformMemory::Free<EAT_Object>(GWorld, sizeof(GWorld));
    GWorld = OriginWorld;
}

void FEditorEngine::Pause()
{
    ULevel* TargetLevel = GEngineLoop.GetWorld()->GetLevel();
    TargetLevel->LevelState = ELevelState::Pause;
}

void FEditorEngine::Resume()
{
    ULevel* TargetLevel = GEngineLoop.GetWorld()->GetLevel();
    TargetLevel->LevelState = ELevelState::Play;;
}
