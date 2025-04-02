#pragma once
#include <sstream>

#include "ViewportClient.h"
#include "Editor/UnrealEd/Editor/EditorEngine.h"


extern FEditorEngine GEngineLoop;


class FEditorViewportClient : public FViewportClient
{
public:
    FEditorViewportClient();
    virtual ~FEditorViewportClient();

    virtual void        Draw(FViewport* Viewport) override;
    virtual UWorld*     GetWorld() const { return NULL; };
    void Initialize(int32 viewportIndex);
    void Tick(float DeltaTime) override;
    void Release();

    void Input();
    void ResizeViewport(const DXGI_SWAP_CHAIN_DESC& swapchaindesc);
    void ResizeViewport(FRect Top, FRect Bottom, FRect Left, FRect Right);

    bool IsSelected(POINT point);


public: 
    int32 ViewportIndex;


public:

    // 카메라 정보 
    /** Viewport's stored horizontal field of view (saved in ini files). */
    float FOVAngle = 60.0f;
    float AspectRatio;
    float nearPlane = 0.1f;
    float farPlane = 1000000.0f;

    FMatrix View;
    FMatrix Projection;
public: //Camera Movement
    void CameraMoveForward(float _Value);
    void CameraMoveRight(float _Value);
    void CameraMoveUp(float _Value);
    void CameraRotateYaw(float _Value);
    void CameraRotatePitch(float _Value);
    void PivotMoveRight(float _Value);
    void PivotMoveUp(float _Value);

    FMatrix& GetViewMatrix() override { return  View; }
    FMatrix& GetProjectionMatrix() override { return Projection; }
    void UpdateViewMatrix();
    void UpdateProjectionMatrix();

    ELevelViewportType GetViewportType() const;
    void SetViewportType(ELevelViewportType InViewportType);
    void UpdateOrthoCameraLoc();
    EViewModeIndex GetViewMode() { return ViewMode; }
    void SetViewMode(EViewModeIndex newMode) { ViewMode = newMode; }
    uint64 GetShowFlag() { return ShowFlag; }
    void SetShowFlag(uint64 newMode) { ShowFlag = newMode; }
    bool GetIsOnRBMouseClick() { return bRightMouseDown; }

private: // Input
    POINT lastMousePos;
    bool bRightMouseDown = false;
   

public:
    void SetCameraSpeedScalar(float value);
private:
    TMap<FString, FString> ReadIniFile(const FString& filePath);
    void WriteIniFile(const FString& filePath, const TMap<FString, FString>& config);

private:
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

