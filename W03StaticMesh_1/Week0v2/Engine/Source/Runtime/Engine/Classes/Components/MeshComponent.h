#pragma once
#include "PrimitiveComponent.h"
#include "Material/Material.h"

class UMeshComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UMeshComponent, UPrimitiveComponent)
public:
    UMeshComponent() = default;

#pragma region Material
    virtual uint32 GetNumMaterials() const { return 0; }
    virtual UMaterial* GetMaterial(uint32 ElementIndex) const;
    virtual uint32 GetMaterialIndex(FName MaterialSlotName) const;
    virtual UMaterial* GetMaterialByName(FName MaterialSlotName) const;
    virtual TArray<FName> GetMaterialSlotNames() const;
    virtual void SetMaterial(uint32 ElementIndex, UMaterial* Material);
    virtual void SetMaterialByName(FName MaterialSlotName, class UMaterial* Material);
    virtual void GetUsedMaterials(TArray<UMaterial*>& Out) const;
#pragma endregion
    // 가상 복사 함수: 기본 UObject 멤버를 복사합니다.
    void CopyPropertiesFrom(UObject* Source) override;
protected:
    TArray<UMaterial*> OverrideMaterials;
public:
    TArray<UMaterial*>& GetOverrideMaterials() { return OverrideMaterials; }
};

