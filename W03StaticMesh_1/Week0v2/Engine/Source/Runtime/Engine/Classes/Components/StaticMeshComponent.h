#pragma once
#include "Components/MeshComponent.h"
#include "Math/JungleMath.h"
#include "Mesh/StaticMesh.h"
#include "Renderer/OcclusionQuery.h"
class UStaticMeshComponent : public UMeshComponent
{
    DECLARE_CLASS(UStaticMeshComponent, UMeshComponent)

public:
    UStaticMeshComponent() = default;

    PROPERTY(int, selectedSubMeshIndex);

    virtual uint32 GetNumMaterials() const override;
    virtual UMaterial* GetMaterial(uint32 ElementIndex) const override;
    virtual uint32 GetMaterialIndex(FName MaterialSlotName) const override;
    virtual TArray<FName> GetMaterialSlotNames() const override;
    virtual void GetUsedMaterials(TArray<UMaterial*>& Out) const override;

    virtual int CheckRayIntersection(const FVector& rayOrigin, const FVector& rayDirection, float& pfNearHitDistance) override;
    
    UStaticMesh* GetStaticMesh() const { return staticMesh; }
    void SetStaticMesh(UStaticMesh* value)
    { 
        staticMesh = value;
        OverrideMaterials.SetNum(value->GetMaterials().Num());
        AABB = FBoundingBox(staticMesh->GetRenderData()->BoundingBoxMin, staticMesh->GetRenderData()->BoundingBoxMax);
        UpdateVertexData();
    }
    FOcclusionQuery query;
    bool bIsVisible = true;
    virtual void OnTransformation() override {
        UpdateVertexData();
    } 

    inline void UpdateVertexData()
    {
        auto LODData = GetStaticMesh()->GetLODDatas();
        for (int level = 0; level < LODData.Num(); ++level)
        {
            LODVertices[level].Empty();
            for (auto& LODVertex : LODData[level]->Vertices)
            {
                FVertexSimple Vertex;

                FMatrix Model = JungleMath::CreateModelMatrix(GetWorldLocation(), GetWorldRotation(), GetWorldScale());

                const FVector Pos = Model.TransformPosition({LODVertex.x, LODVertex.y, LODVertex.z});

                Vertex.x = Pos.x;
                Vertex.y = Pos.y;
                Vertex.z = Pos.z;
                Vertex.u = LODVertex.u;
                Vertex.v = LODVertex.v;
                LODVertices[level].Add(Vertex);
            }
        }
    }

public:
    // 모델 행렬 적용된 VertexData
    TArray<FVertexSimple> Vertices;

    TMap<uint32, TArray<FVertexSimple>> LODVertices;

    void SetLODLevel(const int InLODLevel) { LODLevel = InLODLevel; }
    uint32 GetLODLevel() const { return LODLevel; }
protected:
    UStaticMesh* staticMesh = nullptr;
    uint32 LODLevel = 0;
    int selectedSubMeshIndex = -1;
};