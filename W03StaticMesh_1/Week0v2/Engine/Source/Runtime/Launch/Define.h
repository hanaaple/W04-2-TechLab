#pragma once
#include <cmath>
#include <algorithm>
#include "Core/Container/String.h"
#include "Core/Container/Array.h"
#include "Container/Set.h"
#include "UObject/NameTypes.h"

// 수학 관련
#include "Math/Vector.h"
#include "Math/Vector4.h"
#include "Math/Matrix.h"


#define UE_LOG Console::GetInstance().AddLog

#define _TCHAR_DEFINED
#include <d3d11.h>


#include "UserInterface/Console.h"

struct FVertexSimple
{
    float x, y, z;    // Position
    //float r, g, b, a; // Color
    //float nx, ny, nz;
    float u=0, v=0;
    //uint32 MaterialIndex;
};

// Material Subset
struct FMaterialSubset
{
    uint32 IndexStart; // Index Buffer Start pos
    uint32 IndexCount; // Index Count
    uint32 MaterialIndex; // Material Index
    FString MaterialName; // Material Name
};

struct FStaticMaterial
{
    class UMaterial* Material;
    FName MaterialSlotName;
    //FMeshUVChannelInfo UVChannelData;
};

// OBJ File Raw Data
struct FObjInfo
{
    FWString ObjectName; // OBJ File Name
    FWString PathName; // OBJ File Paths
    FString DisplayName; // Display Name
    FString MatName; // OBJ MTL File Name
    
    // Group
    uint32 NumOfGroup = 0; // token 'g' or 'o'
    TArray<FString> GroupName;
    
    // Vertex, UV, Normal List
    TArray<FVector> Vertices;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    
    // Faces
    TArray<int32> Faces;

    // Index
    TArray<uint32> VertexIndices;
    TArray<uint32> NormalIndices;
    TArray<uint32> TextureIndices;
    
    // Material
    TArray<FMaterialSubset> MaterialSubsets;
};

struct FObjMaterialInfo
{
    FString MTLName;  // newmtl : Material Name.

    bool bHasTexture = false;  // Has Texture?
    bool bTransparent = false; // Has alpha channel?

    FVector Diffuse;  // Kd : Diffuse (Vector4)
    FVector Specular;  // Ks : Specular (Vector) 
    FVector Ambient;   // Ka : Ambient (Vector)
    FVector Emissive;  // Ke : Emissive (Vector)

    float SpecularScalar; // Ns : Specular Power (Float)
    float DensityScalar;  // Ni : Optical Density (Float)
    float TransparencyScalar; // d or Tr  : Transparency of surface (Float)

    uint32 IlluminanceModel; // illum: illumination Model between 0 and 10. (UINT)

    /* Texture */
    FString DiffuseTextureName;  // map_Kd : Diffuse texture
    FWString DiffuseTexturePath;
    
    FString AmbientTextureName;  // map_Ka : Ambient texture
    FWString AmbientTexturePath;
    
    FString SpecularTextureName; // map_Ks : Specular texture
    FWString SpecularTexturePath;
    
    FString BumpTextureName;     // map_Bump : Bump texture
    FWString BumpTexturePath;
    
    FString AlphaTextureName;    // map_d : Alpha texture
    FWString AlphaTexturePath;
};

// Cooked Data
namespace OBJ
{
    struct FStaticMeshRenderData
    {
        FWString ObjectName;
        FWString PathName;
        FString DisplayName;
        
        TArray<FVertexSimple> Vertices;
        TArray<UINT> Indices;

        ID3D11Buffer* VertexBuffer;
        ID3D11Buffer* IndexBuffer;
        
        TArray<FObjMaterialInfo> Materials;
        TArray<FMaterialSubset> MaterialSubsets;

        FVector BoundingBoxMin;
        FVector BoundingBoxMax;
    };

    // 별도의 LOD 데이터 전담 구조체
    //
    struct FStaticMeshLODData
    {
        // LODLevels[0]: 기본(고해상도) 메시, [1]: 중간, [2]: 저해상도 등
        TArray<OBJ::FStaticMeshRenderData*> LODLevels;
    };
}

struct FVertexTexture
{
	float x, y, z;    // Position
	float u, v; // Texture
};
struct FGridParameters
{
	float gridSpacing;
	int   numGridLines;
	FVector gridOrigin;
	float pad;
};
struct FSimpleVertex
{
	float dummy; // 내용은 사용되지 않음
    float padding[11];
};
struct FOBB {
    FVector corners[8];
};
struct FRect
{
    FRect() : leftTopX(0), leftTopY(0), width(0), height(0) {}
    FRect(float x, float y, float w, float h) : leftTopX(x), leftTopY(y), width(w), height(h) {}
    float leftTopX, leftTopY, width, height;
};
struct FPoint
{
    FPoint() : x(0), y(0) {}
    FPoint(float _x, float _y) : x(_x), y(_y) {}
    FPoint(long _x, long _y) : x(_x), y(_y) {}
    FPoint(int _x, int _y) : x(_x), y(_y) {}

    float x, y;
};
struct FBoundingBox
{
public:
    FBoundingBox(){}
    FBoundingBox(FVector _min, FVector _max);

	FVector min; // Minimum extents
	float pad;
	FVector max; // Maximum extents
	float pad1;
    
    bool IntersectRay(const FVector& rayOrigin, const FVector& rayDir, float& outDistance) const;
    bool IntersectLine(const FVector& p1, const FVector& p2) const;

    void ExpandToInclude(const FBoundingBox& Other);

    static FBoundingBox ComputeSceneBoundingBox(const TSet<class AActor*>& SpawnedActors);

    static FBoundingBox TransformBy(const FBoundingBox& localAABB, const FVector& center, const FMatrix& modelMatrix);

    inline FBoundingBox Expanded(const float scale) const {
        return FBoundingBox(min * scale, max * scale);
    }

    FVector GetCenter() const
    {
        return (max + min) * 0.5f;
    }

    FVector GetExtent() const
    {
        return (max - min) * 0.5f;
    }

    FBoundingBox Transform(const FMatrix& mat) const
    {
        FVector corners[8] = {
            FVector(min.x, min.y, min.z),
            FVector(max.x, min.y, min.z),
            FVector(min.x, max.y, min.z),
            FVector(max.x, max.y, min.z),
            FVector(min.x, min.y, max.z),
            FVector(max.x, min.y, max.z),
            FVector(min.x, max.y, max.z),
            FVector(max.x, max.y, max.z)
        };

        FVector transformedMin = mat.TransformPosition(corners[0]);
        FVector transformedMax = transformedMin;

        for (int i = 1; i < 8; ++i)
        {
            FVector p = mat.TransformPosition(corners[i]);
            transformedMin.x = std::min(transformedMin.x, p.x);
            transformedMin.y = std::min(transformedMin.y, p.y);
            transformedMin.z = std::min(transformedMin.z, p.z);

            transformedMax.x = std::max(transformedMax.x, p.x);
            transformedMax.y = std::max(transformedMax.y, p.y);
            transformedMax.z = std::max(transformedMax.z, p.z);
        }

        return FBoundingBox(transformedMin, transformedMax);
    }
};
struct FCone
{
    FVector ConeApex; // 원뿔의 꼭짓점
    float ConeRadius; // 원뿔 밑면 반지름

    FVector ConeBaseCenter; // 원뿔 밑면 중심
    float ConeHeight; // 원뿔 높이 (Apex와 BaseCenter 간 차이)
    FVector4 Color;

    int ConeSegmentCount; // 원뿔 밑면 분할 수
    float pad[3];

};
struct FPrimitiveCounts 
{
	int BoundingBoxCount;
	int pad;
	int ConeCount; 
	int pad1;
};
struct FLighting
{
	float lightDirX, lightDirY, lightDirZ; // 조명 방향
	float pad1;                      // 16바이트 정렬용 패딩
	float lightColorX, lightColorY, lightColorZ;    // 조명 색상
	float pad2;                      // 16바이트 정렬용 패딩
	float AmbientFactor;             // ambient 계수
	float pad3; // 16바이트 정렬 맞춤 추가 패딩
	float pad4; // 16바이트 정렬 맞춤 추가 패딩
	float pad5; // 16바이트 정렬 맞춤 추가 패딩
};

struct alignas(16) FMaterialConstants {
    FVector DiffuseColor;
    // float TransparencyScalar;
    // FVector AmbientColor;
    // float DensityScalar;
    // FVector SpecularColor;
    // float SpecularScalar;
    // FVector EmmisiveColor;
    // float MaterialPad0;
};

struct alignas(16) FConstants {
    FMatrix MVP;      // 모델
    // FMatrix ModelMatrixInverseTranspose; // normal 변환을 위한 행렬
    // FVector4 UUIDColor;
    // bool IsSelected;
};

struct alignas(16) FLitUnlitConstants {
    int isLit; // 1 = Lit, 0 = Unlit 
    FVector pad;
};

struct alignas(16) FSubMeshConstants {
    float isSelectedSubMesh;
    FVector pad;
};

struct alignas(16) FTextureConstants {
    float UOffset;
    float VOffset;
    float pad0;
    float pad1;
};