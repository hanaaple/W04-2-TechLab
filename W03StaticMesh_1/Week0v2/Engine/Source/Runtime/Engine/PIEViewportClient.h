#pragma once
#include "ViewportClient.h"

class UCameraComponent;

class FPIEViewportClient: public FViewportClient
{
public:
    void SetMainCamera(UCameraComponent* camera);
    UCameraComponent* GetMainCamera();
private:
    UCameraComponent* mainCamera;   
};
