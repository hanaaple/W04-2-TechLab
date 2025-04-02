#pragma once
#include "Define.h"
#include "Container/Map.h"
#include "EngineBaseTypes.h"
#include "UObject/ObjectMacros.h"
#include "fstream"
#include "sstream"
#include "ostream"

#define MIN_ORTHOZOOM				1.0							/* 2D ortho viewport zoom >= MIN_ORTHOZOOM */
#define MAX_ORTHOZOOM				1e25

class FViewport;
class UWorld;

struct FViewportCameraTransform
{
private:

public:

    FVector GetForwardVector();
    FVector GetRightVector();
    FVector GetUpVector();

public:
    FViewportCameraTransform();

    /** Sets the transform's location */
    void SetLocation(const FVector& Position)
    {
        ViewLocation = Position;
    }

    /** Sets the transform's rotation */
    void SetRotation(const FVector& Rotation)
    {
        ViewRotation = Rotation;
    }

    /** Sets the location to look at during orbit */
    void SetLookAt(const FVector& InLookAt)
    {
        LookAt = InLookAt;
    }

    /** Set the ortho zoom amount */
    void SetOrthoZoom(float InOrthoZoom)
    {
        assert(InOrthoZoom >= MIN_ORTHOZOOM && InOrthoZoom <= MAX_ORTHOZOOM);
        OrthoZoom = InOrthoZoom;
    }

    /** Check if transition curve is playing. */
 /*    bool IsPlaying();*/

    /** @return The transform's location */
    FORCEINLINE const FVector& GetLocation() const { return ViewLocation; }

    /** @return The transform's rotation */
    FORCEINLINE const FVector& GetRotation() const { return ViewRotation; }

    /** @return The look at point for orbiting */
    FORCEINLINE const FVector& GetLookAt() const { return LookAt; }

    /** @return The ortho zoom amount */
    FORCEINLINE float GetOrthoZoom() const { return OrthoZoom; }

public:
    /** Current viewport Position. */
    FVector	ViewLocation;
    /** Current Viewport orientation; valid only for perspective projections. */
    FVector ViewRotation;
    FVector	DesiredLocation;
    /** When orbiting, the point we are looking at */
    FVector LookAt;
    /** Viewport start location when animating to another location */
    FVector StartLocation;
    /** Ortho zoom amount */
    float OrthoZoom;
};

class FViewportClient
{
public:
    virtual ~FViewportClient() = default;

    // FViewport에서 발생하는 이벤트를 처리하는 가상 함수들
    //virtual void OnInput(const FInputEvent& Event) = 0;
    virtual void Tick(float DeltaTime) = 0;
    virtual void Draw(FViewport* Viewport) = 0;
    virtual UWorld* GetWorld() const { return NULL; }
    virtual FMatrix& GetViewMatrix() = 0;
    virtual FMatrix& GetProjectionMatrix() = 0;

    int32 ViewportIndex;
    
    static FVector Pivot;
    static float orthoSize;

    //카메라
    /** Viewport camera transform data for perspective viewports */
    FViewportCameraTransform		ViewTransformPerspective;
    FViewportCameraTransform        ViewTransformOrthographic;
    
public:
    void LoadConfig(const TMap<FString, FString>& config);
    void SaveConfig(TMap<FString, FString>& config);
    
protected:
    FViewport* Viewport;
    /** Camera speed setting */
    int32 CameraSpeedSetting = 1;
    /** Camera speed scalar */
    float CameraSpeedScalar = 1.0f;
    float GridSize;
    float ViewFOV = 60.0f;
    ELevelViewportType ViewportType;
    EViewModeIndex ViewMode;
    uint64 ShowFlag;
    
public:
    PROPERTY(int32, CameraSpeedSetting);
    PROPERTY(float, CameraSpeedScalar);
    PROPERTY(float, GridSize);
    PROPERTY(float, ViewFOV);
    PROPERTY(ELevelViewportType, ViewportType);
    PROPERTY(EViewModeIndex, ViewMode);
    PROPERTY(uint64, ShowFlag);
    
    FViewport* GetViewport() { return Viewport; }
    D3D11_VIEWPORT& GetD3DViewport();
    bool IsOrtho() const { return !IsPerspective(); };
    bool IsPerspective() const { return ViewMode == LVT_Perspective; };
    static void SetOthoSize(float _Value);
    
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
