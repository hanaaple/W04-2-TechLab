#pragma once
#include "ViewportClient.h"

class UCameraComponent;

class FPIEViewportClient: public FViewportClient
{
public:
    void Tick(float DeltaTime);
    void Draw(FViewport* Viewport);
    FMatrix& GetViewMatrix();
    FMatrix& GetProjectionMatrix();
    void SetMainCamera(UCameraComponent* camera);
    UCameraComponent* GetMainCamera();
private:
    UCameraComponent* mainCamera;   
};
