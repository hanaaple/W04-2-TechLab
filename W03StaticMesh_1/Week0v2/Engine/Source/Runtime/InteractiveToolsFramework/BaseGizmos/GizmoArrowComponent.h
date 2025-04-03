#pragma once
#include "GizmoBaseComponent.h"
#include "UObject/ObjectTypes.h"

class UGizmoArrowComponent : public UGizmoBaseComponent
{
    DECLARE_CLASS(UGizmoArrowComponent, UGizmoBaseComponent)

public:
    UGizmoArrowComponent();
    virtual ~UGizmoArrowComponent() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;

private:
    ARROW_DIR Dir;

public:
    ARROW_DIR GetDir() const { return Dir; }
    void SetDir(ARROW_DIR _Dir) { Dir = _Dir; }
    // 가상 복사 함수: 기본 UObject 멤버를 복사합니다.
    void CopyPropertiesFrom(UObject* Source, TMap<UObject*, UObject*>& DupMap) override;
    void CopyPropertiesTo(UObject* Dest, TMap<UObject*, UObject*>& OutMap) override;
};
