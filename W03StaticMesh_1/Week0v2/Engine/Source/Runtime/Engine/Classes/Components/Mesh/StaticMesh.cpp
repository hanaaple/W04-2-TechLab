#include "StaticMesh.h"
#include "Engine/FLoaderOBJ.h"
#include "UObject/ObjectFactory.h"

UStaticMesh::UStaticMesh()
{

}

UStaticMesh::~UStaticMesh()
{
    if (staticMeshRenderData == nullptr) return;

    if (staticMeshRenderData->VertexBuffer) {
        staticMeshRenderData->VertexBuffer->Release();
        staticMeshRenderData->VertexBuffer = nullptr;
    }

    if (staticMeshRenderData->IndexBuffer) {
        staticMeshRenderData->IndexBuffer->Release();
        staticMeshRenderData->IndexBuffer = nullptr;
    }
}

uint32 UStaticMesh::GetMaterialIndex(FName MaterialSlotName) const
{
    for (uint32 materialIndex = 0; materialIndex < materials.Num(); materialIndex++) {
        if (materials[materialIndex]->MaterialSlotName == MaterialSlotName)
            return materialIndex;
    }

    return -1;
}

void UStaticMesh::GetUsedMaterials(TArray<UMaterial*>& Out) const
{
    for (const FStaticMaterial* Material : materials)
    {
        Out.Emplace(Material->Material);
    }
}

void UStaticMesh::SetData(OBJ::FStaticMeshRenderData* renderData)
{
    staticMeshRenderData = renderData;

    for (int materialIndex = 0; materialIndex < staticMeshRenderData->Materials.Num(); materialIndex++)
    {
        FStaticMaterial* newMaterialSlot = new FStaticMaterial();
        UMaterial* newMaterial = FManagerOBJ::CreateMaterial(staticMeshRenderData->Materials[materialIndex]);

        newMaterialSlot->Material = newMaterial;
        newMaterialSlot->MaterialSlotName = staticMeshRenderData->Materials[materialIndex].MTLName;

        materials.Add(newMaterialSlot);
    }

    // LOD 데이터를 별도 구조체에 생성 (예: 두 단계 LOD 생성)
    // 기본 메시는 LODLevels[0]
    LODData.Add(renderData);
    
    // 예제: reductionFactor 0.5로 중간 해상도, 0.25로 저해상도 생성
    OBJ::FStaticMeshRenderData* LOD1 = FLoaderOBJ::CreateEdgeCollapseLOD(renderData, 0.5f);
    OBJ::FStaticMeshRenderData* LOD2 = FLoaderOBJ::CreateEdgeCollapseLOD(renderData, 0.25f);
    if (LOD1)
        LODData.Add(LOD1);
    if (LOD2)
        LODData.Add(LOD2);
    
    for (const auto lod : LODData)
    {
        const uint32 numVerts = lod->Vertices.Num();
        if (numVerts > 0)
            lod->VertexBuffer = GetEngine().renderer.CreateVertexBuffer(lod->Vertices, numVerts * sizeof(FVertexSimple));
        const uint32 numIndices = lod->Indices.Num();
        if (numIndices > 0)
            lod->IndexBuffer = GetEngine().renderer.CreateIndexBuffer(lod->Indices, numIndices * sizeof(uint32));
    }
}
