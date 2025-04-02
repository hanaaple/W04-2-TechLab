#pragma once
#include "GameFramework/Actor.h"


class AStaticMeshActor : public AActor
{
    DECLARE_CLASS(AStaticMeshActor, AActor)

public:
    AStaticMeshActor();

    UStaticMeshComponent* GetStaticMeshComponent() const { return StaticMeshComponent; }

    // 가상 복사 함수: 기본 UObject 멤버를 복사합니다.
    UObject* Duplicate() override;
private:
    UStaticMeshComponent* StaticMeshComponent = nullptr;
};
