#pragma once
#include <map>

#include "Define.h"
#include "EngineLoop.h"
#include "Container/Map.h"
#include "Container/PriorityQueue.h"
#include "HAL/PlatformType.h"
#include "Math/MathUtility.h"
#include "Serialization/Serializer.h"

class UStaticMesh;
struct FManagerOBJ;
struct OBJ::FStaticMeshRenderData;

struct FLoaderOBJ
{
    // Obj Parsing (*.obj to FObjInfo)
    static bool ParseOBJ(const FString& ObjFilePath, FObjInfo& OutObjInfo);
    
    // Material Parsing (*.obj to MaterialInfo)
    static bool ParseMaterial(FObjInfo& OutObjInfo, OBJ::FStaticMeshRenderData& OutFStaticMesh);
    
    // Convert the Raw data to Cooked data (FStaticMeshRenderData)
    static bool ConvertToStaticMesh(const FObjInfo& RawData, OBJ::FStaticMeshRenderData& OutStaticMesh);

    static bool CreateTextureFromFile(const FWString& Filename);

    static void ComputeBoundingBox(const TArray<FVertexSimple>& InVertices, FVector& OutMinVector, FVector& OutMaxVector);

    static OBJ::FStaticMeshRenderData* CreateSimpleLOD(const OBJ::FStaticMeshRenderData* HighResData);

    static OBJ::FStaticMeshRenderData* CreateEdgeCollapseLOD(const OBJ::FStaticMeshRenderData* HighResData, const float targetReductionRatio);
private:
    struct FEdgeCollapse
    {
        uint32 v1, v2;
        float cost;
        FVector optimalPos;
    };

    struct FCompareEdgeCollapse
    {
        bool operator()(const FEdgeCollapse& A, const FEdgeCollapse& B) const
        {
            return A.cost > B.cost; // 비용이 낮은 것이 우선
        }
    };

    // 구조체: 경계 에지를 표현 (정점 인덱스는 항상 오름차순으로 저장)
    struct FEdgeKey
    {
        uint32 a, b;
        FEdgeKey(uint32 i, uint32 j)
        {
            a = FMath::Min(i, j);
            b = FMath::Max(i, j);
        }
        bool operator==(const FEdgeKey& other) const
        {
            return a== other.a && b == other.b;
        }
    };

    struct FEdgeKeyHash
    {
        std::size_t operator()(const FEdgeKey& k) const
        {
            return std::hash<UINT>()(k.a) ^ std::hash<UINT>()(k.b);
        }
    };
};

struct FManagerOBJ
{
public:
    static OBJ::FStaticMeshRenderData* LoadObjStaticMeshAsset(const FString& PathFileName);
    
    static void CombineMaterialIndex(OBJ::FStaticMeshRenderData& OutFStaticMesh);

    static bool SaveStaticMeshToBinary(const FWString& FilePath, const OBJ::FStaticMeshRenderData& StaticMesh);

    static bool LoadStaticMeshFromBinary(const FWString& FilePath, OBJ::FStaticMeshRenderData& OutStaticMesh);

    static UMaterial* CreateMaterial(FObjMaterialInfo materialInfo);
    static UStaticMesh* CreateStaticMesh(const FString& filePath);
    
    inline static TMap<FString, UMaterial*>& GetMaterials() { return materialMap; }
    inline static UMaterial* GetMaterial(const FString& name) {  return materialMap[name]; }
    
    inline static int GetMaterialNum() { return materialMap.Num(); }
    inline static int GetStaticMeshNum() { return staticMeshMap.Num(); }

    inline static const TMap<FWString, UStaticMesh*>& GetStaticMeshes() { return staticMeshMap; }
    inline static UStaticMesh* GetStaticMesh(const FWString& name) { return staticMeshMap[name]; }
private:
    inline static TMap<FString, OBJ::FStaticMeshRenderData*> ObjStaticMeshMap;
    inline static TMap<FWString, UStaticMesh*> staticMeshMap;
    inline static TMap<FString, UMaterial*> materialMap;
};