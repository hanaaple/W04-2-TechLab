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
        const OBJ::FStaticMeshRenderData* renderData = GetStaticMesh()->GetRenderData();
        Vertices.Empty();
        for (const auto& OriginVertex : renderData->Vertices)
        {
            FVertexSimple Vertex;

            FMatrix Model = JungleMath::CreateModelMatrix(GetWorldLocation(), GetWorldRotation(), GetWorldScale());
                    
            FVector Pos = Model.TransformPosition({OriginVertex.x, OriginVertex.y, OriginVertex.z});

            Vertex.x = Pos.x;
            Vertex.y = Pos.y;
            Vertex.z = Pos.z;
            Vertex.u = OriginVertex.u;
            Vertex.v = OriginVertex.v;
            Vertices.Add(Vertex);
        }
    }

public:
    // 모델 행렬 적용된 VertexData
    TArray<FVertexSimple> Vertices;
    
protected:
    UStaticMesh* staticMesh = nullptr;
    int selectedSubMeshIndex = -1;
};