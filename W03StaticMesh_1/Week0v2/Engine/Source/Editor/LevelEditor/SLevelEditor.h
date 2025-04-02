#pragma once
#include <memory>

#include "Define.h"
#include "ViewportClient.h"
#include "UnrealEd/EditorViewportClient.h"
#include "Container/Map.h"

class SSplitterH;
class SSplitterV;
class UWorld;
class FEditorViewportClient;
class SLevelEditor
{
public:
    SLevelEditor();
    ~SLevelEditor();
    void Initialize();
    void Tick(double deltaTime);
    void Input();
    void Release();
    
    void SelectViewport(POINT point);
    void OnResize();
    void ResizeViewports();
    void OnMultiViewport();
    void OffMultiViewport();
    bool IsMultiViewport();
private:
    bool bInitialize;
    SSplitterH* HSplitter;
    SSplitterV* VSplitter;
    UWorld* World;
    std::shared_ptr<FEditorViewportClient> viewportClients[4];
    std::shared_ptr<FEditorViewportClient> PIEViewportClient;
    std::shared_ptr<FViewportClient> ActiveViewportClient;

    bool bLButtonDown = false;
    bool bRButtonDown = false;
    
    bool bMultiViewportMode;

    POINT lastMousePos;
    float EditorWidth;
    float EditorHeight;

public:
    std::shared_ptr<FEditorViewportClient>* GetViewports() { return viewportClients; }
    std::shared_ptr<FViewportClient> GetActiveViewportClient() const
    {
        return ActiveViewportClient;
    }
    void SetViewportClient(std::shared_ptr<FEditorViewportClient> viewportClient)
    {
        ActiveViewportClient = std::static_pointer_cast<FViewportClient, FEditorViewportClient>(viewportClient);
    }
    void SetViewportClient(int index)
    {
        ActiveViewportClient = std::static_pointer_cast<FViewportClient, FEditorViewportClient>(viewportClients[index]);
    }

    //Save And Load
private:
    const FString IniFilePath = "editor.ini";
public:
    void LoadConfig();
    void SaveConfig();
private:
    TMap<FString, FString> ReadIniFile(const FString& filePath);
    void WriteIniFile(const FString& filePath, const TMap<FString, FString>& config);

    template <typename T>
    T GetValueFromConfig(const TMap<FString, FString>& config, const FString& key, T defaultValue) {
        if (const FString* Value = config.Find(key))
        {
            std::istringstream iss(**Value);
            T value;
            if (iss >> value)
            {
                return value;
            }
        }
        return defaultValue;
    }
};

