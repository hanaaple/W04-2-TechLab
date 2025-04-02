#pragma once
#include "PrimitiveComponent.h"
#include "Engine/Texture.h"

class UBillboardComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UBillboardComponent, UPrimitiveComponent)
    
public:
    UBillboardComponent();
    virtual ~UBillboardComponent() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(
        FVector& rayOrigin,
        FVector& rayDirection, float& pfNearHitDistance
    ) override;

    void SetTexture(FWString _fileName);
    std::shared_ptr<FTexture> GetTexture() const { return Texture;}
    void SetUUIDParent(USceneComponent* _parent);
    FMatrix CreateBillboardMatrix();

    ID3D11Buffer* vertexTextureBuffer;
    ID3D11Buffer* indexTextureBuffer;
    uint32 numVertices;
    uint32 numIndices;
    float finalIndexU = 0.0f;
    float finalIndexV = 0.0f;
    UObject* Duplicate() override;
protected:



    USceneComponent* m_parent = nullptr;

    bool CheckPickingOnNDC(const TArray<FVector>& checkQuad, float& hitDistance);

private:
    void CreateQuadTextureVertexBuffer();

    
private:
    std::shared_ptr<FTexture> Texture;
};
