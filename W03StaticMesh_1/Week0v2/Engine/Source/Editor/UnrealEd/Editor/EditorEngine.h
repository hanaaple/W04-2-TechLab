#pragma once
#include "Engine.h"
#include "Core/HAL/PlatformType.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Engine/ResourceMgr.h"

class UnrealEd;
class UImGuiManager;
class UWorld;
// class FEditorViewportClient;
class SSplitterV;
class SSplitterH;
class SLevelEditor;
class FRenderer;

// TODO 임시로 UObject 상속 안함.
class FEditorEngine : public FEngine
{
    //DECLARE_CLASS(UEditorEngine, UEngine)
public:
    FEditorEngine();

    int32 PreInit();
    int32 Init(HINSTANCE hInstance);
    void Render();
    void Tick();
    void Exit();
    float GetAspectRatio(IDXGISwapChain* swapChain) const;
    void Input();

private:
    void WindowInit(HINSTANCE hInstance);

public:
    static FGraphicsDevice graphicDevice;
    static FRenderer renderer;
    static FResourceMgr resourceMgr;
    static uint32 TotalAllocationBytes;
    static uint32 TotalAllocationCount;


    HWND hWnd;

private:
    UWorld* GWorld;
    UWorld* OriginWorld = nullptr;
    // ImGUI
    UImGuiManager* UIMgr;
    // Slate
    SLevelEditor* LevelEditor;
    UnrealEd* UnrealEditor;
    
    bool bIsExit = false;
    const int32 targetFPS = 60;
    bool bTestInput = false;

public:
    UWorld* GetWorld() const { return GWorld; }
    SLevelEditor* GetLevelEditor() const { return LevelEditor; }
    UnrealEd* GetUnrealEditor() const { return UnrealEditor; }
    void StartPIE();
    void EndPIE();
    void Pause();
    void Resume();
};
