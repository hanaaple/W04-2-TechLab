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
    OBJ::FStaticMeshRenderData* GetRenderData(const uint32 LODLevel = 0) const { return LODData[LODLevel]; }
    TArray<OBJ::FStaticMeshRenderData*> GetLODDatas() const { return LODData; }
    void SetData(OBJ::FStaticMeshRenderData* renderData);

private:
    //OBJ::FStaticMeshRenderData* staticMeshRenderData = nullptr;
    
    // 카메라 정보와 메시의 FStaticMeshRenderData를 이용하여 LOD 레벨을 선택하는 함수
    // 예를 들어, 화면 비율이 50% 이상이면 LOD 0 (고해상도), 10% 이상이면 LOD 1, 그보다 작으면 LOD 2
    TArray<OBJ::FStaticMeshRenderData*> LODData;
    TArray<FStaticMaterial*> materials;
};
