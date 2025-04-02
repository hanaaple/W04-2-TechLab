#include "PIEViewportClient.h"

void FPIEViewportClient::SetMainCamera(UCameraComponent* camera)
{
    mainCamera = camera;
}

UCameraComponent* FPIEViewportClient::GetMainCamera()
{
    return mainCamera;
}
