#include "PIEViewportClient.h"

#include "UnrealClient.h"
#include "Camera/CameraComponent.h"
#include "Math/JungleMath.h"

void FPIEViewportClient::Tick(float DeltaTime)
{
}

void FPIEViewportClient::Draw(FViewport* Viewport)
{
}

FMatrix& FPIEViewportClient::GetViewMatrix()
{
    FMatrix viewMatrix = JungleMath::CreateViewMatrix(
        mainCamera->GetWorldLocation(), 
        mainCamera->GetWorldLocation() + mainCamera->GetForwardVector(),
        FVector(0.f, 0.f, 1.f)
        
    );
    return viewMatrix;
}

FMatrix& FPIEViewportClient::GetProjectionMatrix()
{
    FMatrix Projection;
    if (IsPerspective()) {
        Projection = JungleMath::CreateProjectionMatrix(
            ViewFOV * (3.141592f / 180.0f),
            GetViewport()->GetViewport().Width/ GetViewport()->GetViewport().Height,
            nearPlane,
            farPlane
        );
    }
    else
    {
        // 스왑체인의 가로세로 비율을 구합니다.
        float aspectRatio = GetViewport()->GetViewport().Width / GetViewport()->GetViewport().Height;

        // 오쏘그래픽 너비는 줌 값과 가로세로 비율에 따라 결정됩니다.
        float orthoWidth = orthoSize * aspectRatio;
        float orthoHeight = orthoSize;

        // 오쏘그래픽 투영 행렬 생성 (nearPlane, farPlane 은 기존 값 사용)
        Projection = JungleMath::CreateOrthoProjectionMatrix(
            orthoWidth,
            orthoHeight,
            nearPlane,
            farPlane
        );
    }
    return Projection;
}

void FPIEViewportClient::SetMainCamera(UCameraComponent* camera)
{
    mainCamera = camera;
}

UCameraComponent* FPIEViewportClient::GetMainCamera()
{
    return mainCamera;
}
