#pragma once

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

    // 1) 키 구조체: 기본 생성자 + 복사/비교 가능
    struct FEdgeKey
    {
        uint32_t a, b;

        // 기본 생성자(필수)
        FEdgeKey() : a(0), b(0) {}

        FEdgeKey(uint32_t i, uint32_t j)
        {
            a = std::min(i, j);
            b = std::max(i, j);
        }

        bool operator==(const FEdgeKey& rhs) const
        {
            return a == rhs.a && b == rhs.b;
        }
    };

    // 3) 해시 functor
    struct FEdgeKeyHash
    {
        // 복사/기본 생성자를 명시적으로 default 선언
        FEdgeKeyHash() = default;
        FEdgeKeyHash(const FEdgeKeyHash&) = default;

        size_t operator()(const FEdgeKey& Key) const
        {
            // 간단한 해시 예시
            return std::hash<uint32_t>()(Key.a) ^ (std::hash<uint32_t>()(Key.b) << 1);
        }
    };
public:
    // Obj Parsing (*.obj to FObjInfo)
    static bool ParseOBJ(const FString& ObjFilePath, FObjInfo& OutObjInfo);
    
    // Material Parsing (*.obj to MaterialInfo)
    static bool ParseMaterial(FObjInfo& OutObjInfo, OBJ::FStaticMeshRenderData& OutFStaticMesh);
    
    // Convert the Raw data to Cooked data (FStaticMeshRenderData)
    static bool ConvertToStaticMesh(const FObjInfo& RawData, OBJ::FStaticMeshRenderData& OutStaticMesh);

    static bool CreateTextureFromFile(const FWString& Filename);

    static void ComputeBoundingBox(const TArray<FVertexSimple>& InVertices, FVector& OutMinVector, FVector& OutMaxVector);

    static OBJ::FStaticMeshRenderData* CreateSimpleLOD(const OBJ::FStaticMeshRenderData* HighResData);

    static bool IsBoundaryVertex(uint32 Vertex, const std::unordered_map<FEdgeKey, int32, FEdgeKeyHash>& EdgeCount, TArray<uint32> Indices);
    
    static OBJ::FStaticMeshRenderData* CreateEdgeCollapseLOD(const OBJ::FStaticMeshRenderData* HighResData, const float targetReductionRatio);

    /// 후처리 스티칭 함수
    /// vertices와 indices는 단순화 후 메시 데이터이며, 경계(구멍)를 채워 메시를 연결합니다.
    static void PostProcessStitching(TArray<FVertexSimple>& Vertices, TArray<uint32>& Indices);

    static // 경계 에지들을 입력받아, 각 경계 루프(구멍)를 TArray<TArray<uint32>> 형태로 반환하는 함수
    TArray<TArray<uint32>> FindBoundaryLoops(const TArray<FEdgeKey>& InBoundaryEdges);
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