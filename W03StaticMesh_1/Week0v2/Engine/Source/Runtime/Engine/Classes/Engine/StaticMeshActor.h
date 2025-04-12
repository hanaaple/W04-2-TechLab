#pragma once
#include "GameFramework/Actor.h"
#include "Engine/Classes/Components/StaticMeshComponent.h"

class AStaticMeshActor : public AActor
{
    DECLARE_CLASS(AStaticMeshActor, AActor)

public:
    AStaticMeshActor();

    UStaticMeshComponent* GetStaticMeshComponent() const { return StaticMeshComponent; }


private:
    UStaticMeshComponent* StaticMeshComponent = nullptr;
};
