#pragma once
#include "Components/StaticMeshComponent.h"
#include "Engine/Texture.h"


class USkySphereComponent : public UStaticMeshComponent
{
    DECLARE_CLASS(USkySphereComponent, UStaticMeshComponent)

public:
    USkySphereComponent();
    virtual ~USkySphereComponent() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    float UOffset = 0;
    float VOffset = 0;
    // 가상 복사 함수: 기본 UObject 멤버를 복사합니다.
    void CopyPropertiesFrom(UObject* Source, TMap<UObject*, UObject*>& DupMap) override;
    void CopyPropertiesTo(UObject* Source, TMap<UObject*, UObject*>& OutMap) override;
};
