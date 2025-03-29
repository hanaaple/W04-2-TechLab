#pragma once

#include "EngineLoop.h"
#include "Container/Map.h"
#include "HAL/PlatformType.h"

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

    static OBJ::FStaticMeshRenderData* CreateSimpleLOD(const OBJ::FStaticMeshRenderData* HighResData, float reductionFactor);
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