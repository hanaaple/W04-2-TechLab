#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Core/Container/Array.h"

struct OBJ::FStaticMeshRenderData;
struct FStaticMaterial;

class UStaticMesh : public UObject
{
    DECLARE_CLASS(UStaticMesh, UObject)

public:
    UStaticMesh();
    virtual ~UStaticMesh() override;
    const TArray<FStaticMaterial*>& GetMaterials() const { return materials; }
    uint32 GetMaterialIndex(FName MaterialSlotName) const;
    void GetUsedMaterials(TArray<UMaterial*>& Out) const;
    OBJ::FStaticMeshRenderData* GetRenderData() const { return staticMeshRenderData; }

    void SetData(OBJ::FStaticMeshRenderData* renderData);

private:
    OBJ::FStaticMeshRenderData* staticMeshRenderData = nullptr;
    TArray<OBJ::FStaticMeshRenderData*> LODData;
    TArray<FStaticMaterial*> materials;
};
