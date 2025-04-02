#pragma once
#include "StaticMeshComponent.h"


class USphereComp : public UStaticMeshComponent
{
    DECLARE_CLASS(USphereComp, UStaticMeshComponent)

public:
    USphereComp();
    virtual ~USphereComp() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    // 가상 복사 함수: 기본 UObject 멤버를 복사합니다.
    void CopyPropertiesFrom(UObject* Source, TMap<UObject*, UObject*>& DupMap) override;
};
