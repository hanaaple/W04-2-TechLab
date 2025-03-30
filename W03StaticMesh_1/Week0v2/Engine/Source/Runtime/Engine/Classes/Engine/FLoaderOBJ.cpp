#include "FLoaderOBJ.h"

#include <fstream>
#include <sstream>
#include <stack>

#include "UObject/ObjectFactory.h"
#include "Components/Material/Material.h"
#include "Components/Mesh/StaticMesh.h"
#include "Serialization/Serializer.h"
#include "Core/Container/PriorityQueue.h"
#include "Math/MathUtility.h"

bool FLoaderOBJ::ParseOBJ(const FString& ObjFilePath, FObjInfo& OutObjInfo)
{
    std::ifstream OBJ(ObjFilePath.ToWideString());
    if (!OBJ)
    {
        return false;
    }

    OutObjInfo.PathName = ObjFilePath.ToWideString().substr(0, ObjFilePath.ToWideString().find_last_of(L"\\/") + 1);
    OutObjInfo.ObjectName = ObjFilePath.ToWideString().substr(ObjFilePath.ToWideString().find_last_of(L"\\/") + 1);
    // ObjectName은 wstring 타입이므로, 이를 string으로 변환 (간단한 ASCII 변환의 경우)
    std::wstring wideName = OutObjInfo.ObjectName;
    std::string fileName(wideName.begin(), wideName.end());

    // 마지막 '.'을 찾아 확장자를 제거
    size_t dotPos = fileName.find_last_of('.');
    if (dotPos != std::string::npos) {
        OutObjInfo.DisplayName = fileName.substr(0, dotPos);
    } else {
        OutObjInfo.DisplayName = fileName;
    }
    
    std::string Line;

    while (std::getline(OBJ, Line))
    {
        if (Line.empty() || Line[0] == '#')
            continue;
        
        std::istringstream LineStream(Line);
        std::string Token;
        LineStream >> Token;

        if (Token == "mtllib")
        {
            LineStream >> Line;
            OutObjInfo.MatName = Line;
            continue;
        }

        if (Token == "usemtl")
        {
            LineStream >> Line;
            FString MatName(Line);

            if (!OutObjInfo.MaterialSubsets.IsEmpty())
            {
                FMaterialSubset& LastSubset = OutObjInfo.MaterialSubsets[OutObjInfo.MaterialSubsets.Num() - 1];
                LastSubset.IndexCount = OutObjInfo.VertexIndices.Num() - LastSubset.IndexStart;
            }
            
            FMaterialSubset MaterialSubset;
            MaterialSubset.MaterialName = MatName;
            MaterialSubset.IndexStart = OutObjInfo.VertexIndices.Num();
            MaterialSubset.IndexCount = 0;
            OutObjInfo.MaterialSubsets.Add(MaterialSubset);
        }

        if (Token == "g" || Token == "o")
        {
            LineStream >> Line;
            OutObjInfo.GroupName.Add(Line);
            OutObjInfo.NumOfGroup++;
        }

        if (Token == "v") // Vertex
        {
            float x, y, z;
            LineStream >> x >> y >> z;
            OutObjInfo.Vertices.Add(FVector(x,y,z));
            continue;
        }

        if (Token == "vn") // Normal
        {
            float nx, ny, nz;
            LineStream >> nx >> ny >> nz;
            OutObjInfo.Normals.Add(FVector(nx,ny,nz));
            continue;
        }

        if (Token == "vt") // Texture
        {
            float u, v;
            LineStream >> u >> v;
            OutObjInfo.UVs.Add(FVector2D(u, v));
            continue;
        }

        if (Token == "f")
        {
            TArray<uint32> faceVertexIndices;  // 이번 페이스의 정점 인덱스
            TArray<uint32> faceNormalIndices;  // 이번 페이스의 법선 인덱스
            TArray<uint32> faceTextureIndices; // 이번 페이스의 텍스처 인덱스
            
            while (LineStream >> Token)
            {
                std::istringstream tokenStream(Token);
                std::string part;
                TArray<std::string> facePieces;

                // '/'로 분리하여 v/vt/vn 파싱
                while (std::getline(tokenStream, part, '/'))
                {
                    facePieces.Add(part);
                }

                // OBJ 인덱스는 1부터 시작하므로 -1로 변환
                uint32 vertexIndex = facePieces[0].empty() ? 0 : std::stoi(facePieces[0]) - 1;
                uint32 textureIndex = (facePieces.Num() > 1 && !facePieces[1].empty()) ? std::stoi(facePieces[1]) - 1 : UINT32_MAX;
                uint32 normalIndex = (facePieces.Num() > 2 && !facePieces[2].empty()) ? std::stoi(facePieces[2]) - 1 : UINT32_MAX;

                faceVertexIndices.Add(vertexIndex);
                faceTextureIndices.Add(textureIndex);
                faceNormalIndices.Add(normalIndex);
            }

            if (faceVertexIndices.Num() == 4) // 쿼드
            {
                // 첫 번째 삼각형: 0-1-2
                OutObjInfo.VertexIndices.Add(faceVertexIndices[0]);
                OutObjInfo.VertexIndices.Add(faceVertexIndices[1]);
                OutObjInfo.VertexIndices.Add(faceVertexIndices[2]);

                OutObjInfo.TextureIndices.Add(faceTextureIndices[0]);
                OutObjInfo.TextureIndices.Add(faceTextureIndices[1]);
                OutObjInfo.TextureIndices.Add(faceTextureIndices[2]);

                OutObjInfo.NormalIndices.Add(faceNormalIndices[0]);
                OutObjInfo.NormalIndices.Add(faceNormalIndices[1]);
                OutObjInfo.NormalIndices.Add(faceNormalIndices[2]);

                // 두 번째 삼각형: 0-2-3
                OutObjInfo.VertexIndices.Add(faceVertexIndices[0]);
                OutObjInfo.VertexIndices.Add(faceVertexIndices[2]);
                OutObjInfo.VertexIndices.Add(faceVertexIndices[3]);

                OutObjInfo.TextureIndices.Add(faceTextureIndices[0]);
                OutObjInfo.TextureIndices.Add(faceTextureIndices[2]);
                OutObjInfo.TextureIndices.Add(faceTextureIndices[3]);

                OutObjInfo.NormalIndices.Add(faceNormalIndices[0]);
                OutObjInfo.NormalIndices.Add(faceNormalIndices[2]);
                OutObjInfo.NormalIndices.Add(faceNormalIndices[3]);
            }
            else if (faceVertexIndices.Num() == 3) // 삼각형
            {
                OutObjInfo.VertexIndices.Add(faceVertexIndices[0]);
                OutObjInfo.VertexIndices.Add(faceVertexIndices[1]);
                OutObjInfo.VertexIndices.Add(faceVertexIndices[2]);

                OutObjInfo.TextureIndices.Add(faceTextureIndices[0]);
                OutObjInfo.TextureIndices.Add(faceTextureIndices[1]);
                OutObjInfo.TextureIndices.Add(faceTextureIndices[2]);

                OutObjInfo.NormalIndices.Add(faceNormalIndices[0]);
                OutObjInfo.NormalIndices.Add(faceNormalIndices[1]);
                OutObjInfo.NormalIndices.Add(faceNormalIndices[2]);
            }
            // // 삼각형화 (삼각형 팬 방식)
            // for (int j = 1; j + 1 < faceVertexIndices.Num(); j++)
            // {
            //     OutObjInfo.VertexIndices.Add(faceVertexIndices[0]);
            //     OutObjInfo.VertexIndices.Add(faceVertexIndices[j]);
            //     OutObjInfo.VertexIndices.Add(faceVertexIndices[j + 1]);
            //
            //     OutObjInfo.TextureIndices.Add(faceTextureIndices[0]);
            //     OutObjInfo.TextureIndices.Add(faceTextureIndices[j]);
            //     OutObjInfo.TextureIndices.Add(faceTextureIndices[j + 1]);
            //
            //     OutObjInfo.NormalIndices.Add(faceNormalIndices[0]);
            //     OutObjInfo.NormalIndices.Add(faceNormalIndices[j]);
            //     OutObjInfo.NormalIndices.Add(faceNormalIndices[j + 1]);
            // }
        }
    }

    if (!OutObjInfo.MaterialSubsets.IsEmpty())
    {
        FMaterialSubset& LastSubset = OutObjInfo.MaterialSubsets[OutObjInfo.MaterialSubsets.Num() - 1];
        LastSubset.IndexCount = OutObjInfo.VertexIndices.Num() - LastSubset.IndexStart;
    }
    
    return true;
}

bool FLoaderOBJ::ParseMaterial(FObjInfo& OutObjInfo, OBJ::FStaticMeshRenderData& OutFStaticMesh)
{
    // Subset
    OutFStaticMesh.MaterialSubsets = OutObjInfo.MaterialSubsets;
    
    std::ifstream MtlFile(OutObjInfo.PathName + OutObjInfo.MatName.ToWideString());
    if (!MtlFile.is_open())
    {
        return false;
    }

    std::string Line;
    int32 MaterialIndex = -1;
    
    while (std::getline(MtlFile, Line))
    {
        if (Line.empty() || Line[0] == '#')
            continue;
        
        std::istringstream LineStream(Line);
        std::string Token;
        LineStream >> Token;

        // Create new material if token is 'newmtl'
        if (Token == "newmtl")
        {
            LineStream >> Line;
            MaterialIndex++;

            FObjMaterialInfo Material;
            Material.MTLName = Line;
            OutFStaticMesh.Materials.Add(Material);
        }

        if (Token == "Kd")
        {
            float x, y, z;
            LineStream >> x >> y >> z;
            OutFStaticMesh.Materials[MaterialIndex].Diffuse = FVector(x, y, z);
        }

        if (Token == "Ks")
        {
            float x, y, z;
            LineStream >> x >> y >> z;
            OutFStaticMesh.Materials[MaterialIndex].Specular = FVector(x, y, z);
        }

        if (Token == "Ka")
        {
            float x, y, z;
            LineStream >> x >> y >> z;
            OutFStaticMesh.Materials[MaterialIndex].Ambient = FVector(x, y, z);
        }

        if (Token == "Ke")
        {
            float x, y, z;
            LineStream >> x >> y >> z;
            OutFStaticMesh.Materials[MaterialIndex].Emissive = FVector(x, y, z);
        }

        if (Token == "Ns")
        {
            float x;
            LineStream >> x;
            OutFStaticMesh.Materials[MaterialIndex].SpecularScalar = x;
        }

        if (Token == "Ni")
        {
            float x;
            LineStream >> x;
            OutFStaticMesh.Materials[MaterialIndex].DensityScalar = x;
        }

        if (Token == "d" || Token == "Tr")
        {
            float x;
            LineStream >> x;
            OutFStaticMesh.Materials[MaterialIndex].TransparencyScalar = x;
            OutFStaticMesh.Materials[MaterialIndex].bTransparent = true;
        }

        if (Token == "illum")
        {
            uint32 x;
            LineStream >> x;
            OutFStaticMesh.Materials[MaterialIndex].IlluminanceModel = x;
        }

        if (Token == "map_Kd")
        {
            LineStream >> Line;
            OutFStaticMesh.Materials[MaterialIndex].DiffuseTextureName = Line;

            FWString TexturePath = OutObjInfo.PathName + OutFStaticMesh.Materials[MaterialIndex].DiffuseTextureName.ToWideString();
            OutFStaticMesh.Materials[MaterialIndex].DiffuseTexturePath = TexturePath;
            OutFStaticMesh.Materials[MaterialIndex].bHasTexture = true;

            CreateTextureFromFile(OutFStaticMesh.Materials[MaterialIndex].DiffuseTexturePath);
        }
    }
    
    return true;
}

bool FLoaderOBJ::ConvertToStaticMesh(const FObjInfo& RawData, OBJ::FStaticMeshRenderData& OutStaticMesh)
{
    OutStaticMesh.ObjectName = RawData.ObjectName;
    OutStaticMesh.PathName = RawData.PathName;
    OutStaticMesh.DisplayName = RawData.DisplayName;

    // 고유 정점을 기반으로 FVertexSimple 배열 생성
    TMap<std::string, uint32> vertexMap; // 중복 체크용

    for (int32 i = 0; i < RawData.VertexIndices.Num(); i++)
    {
        uint32 vIdx = RawData.VertexIndices[i];
        uint32 tIdx = RawData.TextureIndices[i];
        uint32 nIdx = RawData.NormalIndices[i];

        // 키 생성 (v/vt/vn 조합)
        std::string key = std::to_string(vIdx) + "/" + 
                         std::to_string(tIdx) + "/" + 
                         std::to_string(nIdx);

        uint32 index;
        if (vertexMap.Find(key) == nullptr)
        {
            FVertexSimple vertex {};
            vertex.x = RawData.Vertices[vIdx].x;
            vertex.y = RawData.Vertices[vIdx].y;
            vertex.z = RawData.Vertices[vIdx].z;

            //vertex.r = 1.0f; vertex.g = 1.0f; vertex.b = 1.0f; vertex.a = 1.0f; // 기본 색상

            if (tIdx != UINT32_MAX && tIdx < RawData.UVs.Num())
            {
                vertex.u = RawData.UVs[tIdx].x;
                vertex.v = -RawData.UVs[tIdx].y;
            }

            // if (nIdx != UINT32_MAX && nIdx < RawData.Normals.Num())
            // {
            //     vertex.nx = RawData.Normals[nIdx].x;
            //     vertex.ny = RawData.Normals[nIdx].y;
            //     vertex.nz = RawData.Normals[nIdx].z;
            // }

            // for (int32 j = 0; j < OutStaticMesh.MaterialSubsets.Num(); j++)
            // {
            //     const FMaterialSubset& subset = OutStaticMesh.MaterialSubsets[j];
            //     if ( i >= subset.IndexStart && i < subset.IndexStart + subset.IndexCount)
            //     {
            //         vertex.MaterialIndex = subset.MaterialIndex;
            //         break;
            //     }
            // }
            
            index = OutStaticMesh.Vertices.Num();
            OutStaticMesh.Vertices.Add(vertex);
            vertexMap[key] = index;
        }
        else
        {
            index = vertexMap[key];
        }

        OutStaticMesh.Indices.Add(index);
        
    }

    // Calculate StaticMesh BoundingBox
    ComputeBoundingBox(OutStaticMesh.Vertices, OutStaticMesh.BoundingBoxMin, OutStaticMesh.BoundingBoxMax);
    
    return true;
} 

bool FLoaderOBJ::CreateTextureFromFile(const FWString& Filename)
{
    
    if (FEngineLoop::resourceMgr.GetTexture(Filename))
    {
        return true;
    }

    HRESULT hr = FEngineLoop::resourceMgr.LoadTextureFromFile(FEngineLoop::graphicDevice.Device, FEngineLoop::graphicDevice.DeviceContext, Filename.c_str());

    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

void FLoaderOBJ::ComputeBoundingBox(const TArray<FVertexSimple>& InVertices, FVector& OutMinVector, FVector& OutMaxVector)
{
    FVector MinVector = { FLT_MAX, FLT_MAX, FLT_MAX };
    FVector MaxVector = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
    
    for (int32 i = 0; i < InVertices.Num(); i++)
    {
        MinVector.x = std::min(MinVector.x, InVertices[i].x);
        MinVector.y = std::min(MinVector.y, InVertices[i].y);
        MinVector.z = std::min(MinVector.z, InVertices[i].z);

        MaxVector.x = std::max(MaxVector.x, InVertices[i].x);
        MaxVector.y = std::max(MaxVector.y, InVertices[i].y);
        MaxVector.z = std::max(MaxVector.z, InVertices[i].z);
    }

    OutMinVector = MinVector;
    OutMaxVector = MaxVector;
}

OBJ::FStaticMeshRenderData* FLoaderOBJ::CreateSimpleLOD(const OBJ::FStaticMeshRenderData* HighResData)
{
    // LODData 생성 및 메타 정보 복사
    OBJ::FStaticMeshRenderData* LODData = new OBJ::FStaticMeshRenderData();
    LODData->ObjectName  = HighResData->ObjectName;
    LODData->PathName    = HighResData->PathName;
    LODData->DisplayName = HighResData->DisplayName;
    LODData->Materials   = HighResData->Materials;
    LODData->MaterialSubsets.Empty(); // 여기서는 별도의 재질 그룹핑은 하지 않음

    // 원본 메시의 정점, 인덱스 배열 참조
    const TArray<FVertexSimple>& srcVertices = HighResData->Vertices;
    const TArray<uint32>& srcIndices = HighResData->Indices;
    const size_t triCount = srcIndices.Num() / 3;

    // 각 삼각형의 centroid를 계산하여 새로운 vertex 배열에 저장
    TArray<FVertexSimple> centroids;
    centroids.Reserve(triCount);
    for (size_t tri = 0; tri < triCount; tri++)
    {
        const size_t base = tri * 3;
        const uint32 i0 = srcIndices[base];
        const uint32 i1 = srcIndices[base + 1];
        const uint32 i2 = srcIndices[base + 2];

        // INVALID marker 체크
        if (i0 == std::numeric_limits<uint32>::max() ||
            i1 == std::numeric_limits<uint32>::max() ||
            i2 == std::numeric_limits<uint32>::max())
        {
            continue;
        }

        const FVertexSimple& v0 = srcVertices[i0];
        const FVertexSimple& v1 = srcVertices[i1];
        const FVertexSimple& v2 = srcVertices[i2];

        FVertexSimple centroid;
        centroid.x = (v0.x + v1.x + v2.x) / 3.0f;
        centroid.y = (v0.y + v1.y + v2.y) / 3.0f;
        centroid.z = (v0.z + v1.z + v2.z) / 3.0f;
        centroid.u = (v0.u + v1.u + v2.u) / 3.0f;
        centroid.v = (v0.v + v1.v + v2.v) / 3.0f;
        // UV 보간: 각 정점의 uv 평균
        centroid.u = (v0.u + v1.u + v2.u) / 3.0f;
        centroid.v = (v0.v + v1.v + v2.v) / 3.0f;
        // (필요하다면 법선 등 추가 보간)

        centroids.Add(centroid);
    }
    
    if (centroids.Num() < 3)
    {
        // 충분한 삼각형이 없다면 LOD 생성 불가
        return nullptr;
    }

    // 전체 중심(global centroid) 계산: 모든 삼각형 centroid의 평균
    FVector globalCentroid = { 0, 0, 0 };
    for (const FVertexSimple& c : centroids)
    {
        globalCentroid.x += c.x;
        globalCentroid.y += c.y;
        globalCentroid.z += c.z;
    }
    const float invCount = 1.0f / centroids.Num();
    globalCentroid.x *= invCount;
    globalCentroid.y *= invCount;
    globalCentroid.z *= invCount;

    // 각 centroid에 대해, globalCentroid를 기준으로 XY 평면 상의 각도를 계산
    struct CentroidAngle {
        int index;
        float angle;
    };
    
    TArray<CentroidAngle> centroidAngles;
    centroidAngles.Reserve(centroids.Num());
    for (int i = 0; i < centroids.Num(); i++)
    {
        const float dx = centroids[i].x - globalCentroid.x;
        const float dy = centroids[i].y - globalCentroid.y;
        const float angle = std::atan2(dy, dx);
        CentroidAngle ca = { i, angle };
        centroidAngles.Add(ca);
    }
    // 정렬: 각도 오름차순
    std::sort(centroidAngles.begin(), centroidAngles.end(), [](const CentroidAngle& a, const CentroidAngle& b) {
        return a.angle < b.angle;
    });

    // 정렬된 순서대로 새로운 vertex 배열 구성
    TArray<FVertexSimple> newVertices;
    newVertices.SetNum(centroids.Num());
    for (int i = 0; i < centroidAngles.Num(); i++)
    {
        newVertices[i] = centroids[centroidAngles[i].index];
    }

    // 삼각형 팬을 이용한 인덱스 배열 생성
    // 첫 번째 정점을 중심으로 사용하고, 나머지 정점들을 순서대로 연결
    TArray<uint32> newIndices;
    // 최소 3개의 정점이 있어야 삼각형 팬 생성 가능
    for (int i = 1; i < newVertices.Num() - 1; i++)
    {
        newIndices.Add(0);     // 중심
        newIndices.Add(i);     // 현재 정점
        newIndices.Add(i + 1); // 다음 정점
    }

    // MaterialSubset 재구성: 여기서는 전체 LOD를 단일 재질 그룹으로 설정
    TArray<FMaterialSubset> newMaterialSubsets;
    if (LODData->Materials.Num() > 0)
    {
        FMaterialSubset subset;
        subset.MaterialIndex = 0;  // 첫 번째 재질 사용
        subset.MaterialName = LODData->Materials[0].MTLName;
        subset.IndexStart = 0;
        subset.IndexCount = newIndices.Num();  // 전체 인덱스 범위
        newMaterialSubsets.Add(subset);
    }

    // 결과 LODData 구성
    LODData->Vertices = newVertices;
    LODData->Indices = newIndices;
    LODData->MaterialSubsets = newMaterialSubsets;

    return LODData;
}

OBJ::FStaticMeshRenderData* FLoaderOBJ::CreateEdgeCollapseLOD(const OBJ::FStaticMeshRenderData* HighResData, const float targetReductionRatio)
{
    if (targetReductionRatio <= 0.0f || targetReductionRatio >= 1.0f)
        return nullptr;

    // LODData 생성 및 메타 정보 복사
    OBJ::FStaticMeshRenderData* LODData = new OBJ::FStaticMeshRenderData();
    LODData->ObjectName  = HighResData->ObjectName;
    LODData->PathName    = HighResData->PathName;
    LODData->DisplayName = HighResData->DisplayName;
    LODData->Materials   = HighResData->Materials;
    LODData->MaterialSubsets.Empty();

    // 고해상도 데이터를 복사
    TArray<FVertexSimple> vertices = HighResData->Vertices;
    TArray<uint32> indices = HighResData->Indices;
    TArray<FMaterialSubset> newMaterialSubsets;

    // 고해상도 메시의 MaterialSubsets를 기반으로, 각 삼각형의 재질 인덱스를 구성
    TArray<int32> triangleMaterials;
    for (const FMaterialSubset& subset : HighResData->MaterialSubsets)
    {
        int32 numTriangles = subset.IndexCount / 3;
        for (int32 i = 0; i < numTriangles; i++)
        {
            triangleMaterials.Add(subset.MaterialIndex);
        }
    }
    
    uint32 originalTriangleCount = indices.Num() / 3;
    uint32 targetTriangleCount = originalTriangleCount * targetReductionRatio;
    targetTriangleCount = FMath::Max(2u, targetTriangleCount); // 최소 2개의 삼각형 유지

    uint32 originalVertexCount = vertices.Num();
    uint32 targetVertexCount = originalVertexCount * targetReductionRatio;
    targetVertexCount = FMath::Max(4u, targetVertexCount); // 최소 4개의 정점 유지

    // 각 정점의 Quadric 행렬 계산
    // 이 코드는 Quadric Error Metric (QEM) 을 계산하는 부분입니다.
    // 즉, 메시의 각 삼각형에 대해 평면 방정식을 구한 뒤, 그 평면의 quadric 행렬(Q)을 계산하여,
    // 각 삼각형에 속하는 정점들의 quadric에 누적시키는 작업을 합니다.
    // 각 삼각형에 대해 계산된 quadric 행렬(Q)을 삼각형에 포함된 각 정점의 quadric에 더합니다.
    // 이렇게 함으로써, 각 정점의 quadric은 해당 정점이 포함된 모든 삼각형의 평면 오차(metric)를 반영하게 됩니다.
    // 나중에 Edge Collapse 알고리즘에서 두 정점을 병합할 때,
    // 이 quadric 값을 사용하여 병합 후의 오차(cost)를 계산하고,
    // 최적의 정점 위치(optimal position)를 결정하는 데 활용됩니다.
    TArray<FMatrix> vertexQuadrics;
    vertexQuadrics.SetNum(vertices.Num());
    for (size_t i = 0; i < indices.Num(); i += 3)
    {
        uint32 idx0 = indices[i], idx1 = indices[i+1], idx2 = indices[i+2];
        FVertexSimple& v0 = vertices[idx0];
        FVertexSimple& v1 = vertices[idx1];
        FVertexSimple& v2 = vertices[idx2];

        FVector p0 = {v0.x, v0.y, v0.z};
        FVector p1 = {v1.x, v1.y, v1.z};
        FVector p2 = {v2.x, v2.y, v2.z};

        FVector edge1 = p1 - p0;
        FVector edge2 = p2 - p0;
        FVector normal = edge1.Cross(edge2);
        float len = normal.Magnitude();
        if (len < 1e-6f) continue;

        normal.x /= len;
        normal.y /= len;
        normal.z /= len;

        float d = -normal.Dot(p0);
        FMatrix q = FMatrix::ComputePlaneQuadric(normal, d);
        
        vertexQuadrics[idx0] = vertexQuadrics[idx0] + q;
        vertexQuadrics[idx1] = vertexQuadrics[idx1] + q;
        vertexQuadrics[idx2] = vertexQuadrics[idx2] + q;
    }

    TSet<FEdgeKey, FEdgeKeyHash> edgeSet;
    for (size_t i = 0; i < indices.Num(); i += 3)
    {
        UINT idx0 = indices[i], idx1 = indices[i + 1], idx2 = indices[i + 2];
        edgeSet.Add(FEdgeKey(idx0, idx1));
        edgeSet.Add(FEdgeKey(idx1, idx2));
        edgeSet.Add(FEdgeKey(idx2, idx0));
    }

    TArray<FEdgeCollapse> candidateEdges;
    for (const auto& ek : edgeSet)
    {
        FEdgeCollapse ec;
        ec.v1 = ek.a;
        ec.v2 = ek.b;
        // 최적 위치는 여기서는 두 정점의 단순 평균으로 계산합니다.
        FVertexSimple& va = vertices[ec.v1];
        FVertexSimple& vb = vertices[ec.v2];
        ec.optimalPos = { (va.x + vb.x) * 0.5f, (va.y + vb.y) * 0.5f, (va.z + vb.z) * 0.5f };
        FMatrix qSum = vertexQuadrics[ec.v1] + vertexQuadrics[ec.v2];
        ec.cost = FMatrix::EvaluateQuadric(qSum, ec.optimalPos);
        candidateEdges.Add(ec);
    }

    // 3. 반복적 에지 붕괴
    constexpr size_t maxIterations = 1000;
    size_t currentTriangleCount = indices.Num() / 3;
    size_t currentVertexCount = vertices.Num();
    float maxError = 0.0f;

    for (size_t iter = 0; iter < maxIterations; iter++)
    {
        if (currentTriangleCount <= targetTriangleCount || currentVertexCount <= targetVertexCount)
            break;

        // 후보 에지 중 최소 비용 에지를 선택합니다.
        auto it = std::min_element(candidateEdges.begin(), candidateEdges.end(),
            [](const FEdgeCollapse& a, const FEdgeCollapse& b) { return a.cost < b.cost; });
        if (it == candidateEdges.end())
            break;
        FEdgeCollapse bestEdge = *it;
        candidateEdges.RemoveAt(it - candidateEdges.begin());

        // 에지 붕괴: bestEdge.v2를 bestEdge.v1에 합칩니다.
        UINT vKeep = bestEdge.v1;
        UINT vRemove = bestEdge.v2;
        // bestEdge.optimalPos로 정점 위치 업데이트
        vertices[vKeep].x = bestEdge.optimalPos.x;
        vertices[vKeep].y = bestEdge.optimalPos.y;
        vertices[vKeep].z = bestEdge.optimalPos.z;
        // 두 정점의 quadric을 합칩니다.
        vertexQuadrics[vKeep] = vertexQuadrics[vKeep] + vertexQuadrics[vRemove];

        // vRemove는 제거된 것으로 처리 (여기서는 NaN 값으로 마킹)
        vertices[vRemove].x = vertices[vRemove].y = vertices[vRemove].z = std::numeric_limits<float>::quiet_NaN();

        // 인덱스 목록 업데이트: vRemove를 vKeep으로 대체하고, 축퇴된 삼각형은 제거
        TArray<uint32> newIndices;
        TArray<int32> newTriangleMaterials;
        for (size_t i = 0; i < indices.Num(); i += 3)
        {
            UINT i0 = indices[i], i1 = indices[i + 1], i2 = indices[i + 2];
            int triMat = triangleMaterials[i / 3];
            if (i0 == vRemove) i0 = vKeep;
            if (i1 == vRemove) i1 = vKeep;
            if (i2 == vRemove) i2 = vKeep;
            if (i0 == i1 || i1 == i2 || i0 == i2)
                continue; // 축퇴 삼각형 제거
            newIndices.Add(i0);
            newIndices.Add(i1);
            newIndices.Add(i2);
            newTriangleMaterials.Add(triMat);
        }
        indices = newIndices;
        triangleMaterials = newTriangleMaterials;
        currentTriangleCount = indices.Num() / 3;
        currentVertexCount--;  // 단순화를 위해 정점 수 감소

        // 4. 후보 에지 목록 업데이트
        TArray<FEdgeCollapse> newCandidates;
        for (auto& edge : candidateEdges)
        {
            // vRemove 관련 에지는 모두 버립니다.
            if (edge.v1 == vRemove || edge.v2 == vRemove)
                continue;
            // vKeep와 관련된 에지의 경우, 비용 재계산
            if (edge.v1 == vKeep || edge.v2 == vKeep)
            {
                UINT a = edge.v1, b = edge.v2;
                FVertexSimple& va = vertices[a];
                FVertexSimple& vb = vertices[b];
                if (std::isnan(va.x) || std::isnan(vb.x))
                    continue;
                FVector opt = { (va.x + vb.x) * 0.5f, (va.y + vb.y) * 0.5f, (va.z + vb.z) * 0.5f };
                FMatrix qSum = vertexQuadrics[a] + vertexQuadrics[b];
                float cost = FMatrix::EvaluateQuadric(qSum, opt);
                edge.optimalPos = opt;
                edge.cost = cost;
            }
            newCandidates.Add(edge);
        }
        candidateEdges = newCandidates;
        
        maxError = FMath::Max(maxError, bestEdge.cost);
    }

    if (vertices.IsEmpty() || indices.IsEmpty())
    {
        LODData->Indices = HighResData->Indices;
        LODData->Vertices = HighResData->Vertices;
        LODData->Materials = HighResData->Materials;
        LODData->DisplayName = HighResData->DisplayName;
        LODData->IndexBuffer = HighResData->IndexBuffer;
        LODData->VertexBuffer = HighResData->VertexBuffer;
        LODData->MaterialSubsets = HighResData->MaterialSubsets;
        LODData->ObjectName = HighResData->ObjectName;
        LODData->PathName = HighResData->PathName;
        LODData->BoundingBoxMax = HighResData->BoundingBoxMax;
        LODData->BoundingBoxMin = HighResData->BoundingBoxMin;
        return LODData;
    }

    LODData->Indices = indices;
    LODData->Vertices = vertices;

    // 5. 재질(서브셋) 재구성
    // simplified indices와 triangleMaterials를 이용해 재질별 인덱스 그룹을 구성합니다.
    TMap<int32, TArray<uint32>> materialToIndices;
    int32 numTriangles = indices.Num() / 3;
    for (int32 tri = 0; tri < numTriangles; tri++)
    {
        int32 mat = triangleMaterials[tri];
        int32 idx = tri * 3;
        if (!materialToIndices.Contains(mat))
        {
            materialToIndices.Add(mat, TArray<uint32>());
        }
        materialToIndices[mat].Add(indices[idx]);
        materialToIndices[mat].Add(indices[idx+1]);
        materialToIndices[mat].Add(indices[idx+2]);
    }

    TArray<uint32> finalIndices;
    LODData->MaterialSubsets.Empty();
    uint32 currentIndexOffset = 0;
    for (auto& pair : materialToIndices)
    {
        int32 matIndex = pair.Key;
        TArray<uint32>& matIndices = pair.Value;

        FMaterialSubset subset;
        subset.IndexStart = currentIndexOffset; // 누적된 인덱스 오프셋
        subset.IndexCount = matIndices.Num();
        subset.MaterialIndex = matIndex;
        // 재질 이름은 Materials 배열에 있는 정보에서 가져오도록 합니다.
        if (HighResData->Materials.IsValidIndex(matIndex))
        {
            subset.MaterialName = HighResData->Materials[matIndex].MTLName;
        }
        else
        {
            subset.MaterialName = FString();
        }

        finalIndices.Append(matIndices);
        currentIndexOffset += subset.IndexCount;
        LODData->MaterialSubsets.Add(subset);
    }
    indices = finalIndices;
    LODData->Indices = indices;
    

    return LODData;
}

OBJ::FStaticMeshRenderData* FManagerOBJ::LoadObjStaticMeshAsset(const FString& PathFileName)
{
    OBJ::FStaticMeshRenderData* NewStaticMesh = new OBJ::FStaticMeshRenderData();
    
    if ( const auto It = ObjStaticMeshMap.Find(PathFileName))
    {
        return *It;
    }
    
    FWString BinaryPath = (PathFileName + ".bin").ToWideString();
    if (std::ifstream(BinaryPath).good())
    {
        if (LoadStaticMeshFromBinary(BinaryPath, *NewStaticMesh))
        {
            ObjStaticMeshMap.Add(PathFileName, NewStaticMesh);
            return NewStaticMesh;
        }
    }
    
    // Parse OBJ
    FObjInfo NewObjInfo;
    bool Result = FLoaderOBJ::ParseOBJ(PathFileName, NewObjInfo);

    if (!Result)
    {
        delete NewStaticMesh;
        return nullptr;
    }

    // Material
    if (NewObjInfo.MaterialSubsets.Num() > 0)
    {
        Result = FLoaderOBJ::ParseMaterial(NewObjInfo, *NewStaticMesh);

        if (!Result)
        {
            delete NewStaticMesh;
            return nullptr;
        }

        CombineMaterialIndex(*NewStaticMesh);

        for (int materialIndex = 0; materialIndex < NewStaticMesh->Materials.Num(); materialIndex++) {
            CreateMaterial(NewStaticMesh->Materials[materialIndex]);
        }
    }
    
    // Convert FStaticMeshRenderData
    Result = FLoaderOBJ::ConvertToStaticMesh(NewObjInfo, *NewStaticMesh);
    if (!Result)
    {
        delete NewStaticMesh;
        return nullptr;
    }

    SaveStaticMeshToBinary(BinaryPath, *NewStaticMesh);
    ObjStaticMeshMap.Add(PathFileName, NewStaticMesh);
    return NewStaticMesh;
}

void FManagerOBJ::CombineMaterialIndex(OBJ::FStaticMeshRenderData& OutFStaticMesh)
{
    for (int32 i = 0; i < OutFStaticMesh.MaterialSubsets.Num(); i++)
    {
        FString MatName = OutFStaticMesh.MaterialSubsets[i].MaterialName;
        for (int32 j = 0; j < OutFStaticMesh.Materials.Num(); j++)
        {
            if (OutFStaticMesh.Materials[j].MTLName == MatName)
            {
                OutFStaticMesh.MaterialSubsets[i].MaterialIndex = j;
                break;
            }
        }
    }
}

bool FManagerOBJ::SaveStaticMeshToBinary(const FWString& FilePath, const OBJ::FStaticMeshRenderData& StaticMesh)
{
    std::ofstream File(FilePath, std::ios::binary);
    if (!File.is_open())
    {
        assert("CAN'T SAVE STATIC MESH BINARY FILE");
        return false;
    }

    // Object Name
    Serializer::WriteFWString(File, StaticMesh.ObjectName);

    // Path Name
    Serializer::WriteFWString(File, StaticMesh.PathName);

    // Display Name
    Serializer::WriteFString(File, StaticMesh.DisplayName);

    // Vertices
    uint32 VertexCount = StaticMesh.Vertices.Num();
    File.write(reinterpret_cast<const char*>(&VertexCount), sizeof(VertexCount));
    File.write(reinterpret_cast<const char*>(StaticMesh.Vertices.GetData()), VertexCount * sizeof(FVertexSimple));

    // Indices
    uint32 IndexCount = StaticMesh.Indices.Num();
    File.write(reinterpret_cast<const char*>(&IndexCount), sizeof(IndexCount));
    File.write(reinterpret_cast<const char*>(StaticMesh.Indices.GetData()), IndexCount * sizeof(UINT));

    // Materials
    uint32 MaterialCount = StaticMesh.Materials.Num();
    File.write(reinterpret_cast<const char*>(&MaterialCount), sizeof(MaterialCount));
    for (const FObjMaterialInfo& Material : StaticMesh.Materials)
    {
        Serializer::WriteFString(File, Material.MTLName);
        File.write(reinterpret_cast<const char*>(&Material.bHasTexture), sizeof(Material.bHasTexture));
        File.write(reinterpret_cast<const char*>(&Material.bTransparent), sizeof(Material.bTransparent));
        File.write(reinterpret_cast<const char*>(&Material.Diffuse), sizeof(Material.Diffuse));
        File.write(reinterpret_cast<const char*>(&Material.Specular), sizeof(Material.Specular));
        File.write(reinterpret_cast<const char*>(&Material.Ambient), sizeof(Material.Ambient));
        File.write(reinterpret_cast<const char*>(&Material.Emissive), sizeof(Material.Emissive));
        File.write(reinterpret_cast<const char*>(&Material.SpecularScalar), sizeof(Material.SpecularScalar));
        File.write(reinterpret_cast<const char*>(&Material.DensityScalar), sizeof(Material.DensityScalar));
        File.write(reinterpret_cast<const char*>(&Material.TransparencyScalar), sizeof(Material.TransparencyScalar));
        File.write(reinterpret_cast<const char*>(&Material.IlluminanceModel), sizeof(Material.IlluminanceModel));

        Serializer::WriteFString(File, Material.DiffuseTextureName);
        Serializer::WriteFWString(File, Material.DiffuseTexturePath);
        Serializer::WriteFString(File, Material.AmbientTextureName);
        Serializer::WriteFWString(File, Material.AmbientTexturePath);
        Serializer::WriteFString(File, Material.SpecularTextureName);
        Serializer::WriteFWString(File, Material.SpecularTexturePath);
        Serializer::WriteFString(File, Material.BumpTextureName);
        Serializer::WriteFWString(File, Material.BumpTexturePath);
        Serializer::WriteFString(File, Material.AlphaTextureName);
        Serializer::WriteFWString(File, Material.AlphaTexturePath);
    }

    // Material Subsets
    uint32 SubsetCount = StaticMesh.MaterialSubsets.Num();
    File.write(reinterpret_cast<const char*>(&SubsetCount), sizeof(SubsetCount));
    for (const FMaterialSubset& Subset : StaticMesh.MaterialSubsets)
    {
        Serializer::WriteFString(File, Subset.MaterialName);
        File.write(reinterpret_cast<const char*>(&Subset.IndexStart), sizeof(Subset.IndexStart));
        File.write(reinterpret_cast<const char*>(&Subset.IndexCount), sizeof(Subset.IndexCount));
        File.write(reinterpret_cast<const char*>(&Subset.MaterialIndex), sizeof(Subset.MaterialIndex));
    }

    // Bounding Box
    File.write(reinterpret_cast<const char*>(&StaticMesh.BoundingBoxMin), sizeof(FVector));
    File.write(reinterpret_cast<const char*>(&StaticMesh.BoundingBoxMax), sizeof(FVector));
    
    File.close();
    return true;
}

bool FManagerOBJ::LoadStaticMeshFromBinary(const FWString& FilePath, OBJ::FStaticMeshRenderData& OutStaticMesh)
{
    std::ifstream File(FilePath, std::ios::binary);
    if (!File.is_open())
    {
        assert("CAN'T OPEN STATIC MESH BINARY FILE");
        return false;
    }

    TArray<FWString> Textures;

    // Object Name
    Serializer::ReadFWString(File, OutStaticMesh.ObjectName);

    // Path Name
    Serializer::ReadFWString(File, OutStaticMesh.PathName);

    // Display Name
    Serializer::ReadFString(File, OutStaticMesh.DisplayName);

    // Vertices
    uint32 VertexCount = 0;
    File.read(reinterpret_cast<char*>(&VertexCount), sizeof(VertexCount));
    OutStaticMesh.Vertices.SetNum(VertexCount);
    File.read(reinterpret_cast<char*>(OutStaticMesh.Vertices.GetData()), VertexCount * sizeof(FVertexSimple));

    // Indices
    uint32 IndexCount = 0;
    File.read(reinterpret_cast<char*>(&IndexCount), sizeof(IndexCount));
    OutStaticMesh.Indices.SetNum(IndexCount);
    File.read(reinterpret_cast<char*>(OutStaticMesh.Indices.GetData()), IndexCount * sizeof(UINT));

    // Material
    uint32 MaterialCount = 0;
    File.read(reinterpret_cast<char*>(&MaterialCount), sizeof(MaterialCount));
    OutStaticMesh.Materials.SetNum(MaterialCount);
    for (FObjMaterialInfo& Material : OutStaticMesh.Materials)
    {
        Serializer::ReadFString(File, Material.MTLName);
        File.read(reinterpret_cast<char*>(&Material.bHasTexture), sizeof(Material.bHasTexture));
        File.read(reinterpret_cast<char*>(&Material.bTransparent), sizeof(Material.bTransparent));
        File.read(reinterpret_cast<char*>(&Material.Diffuse), sizeof(Material.Diffuse));
        File.read(reinterpret_cast<char*>(&Material.Specular), sizeof(Material.Specular));
        File.read(reinterpret_cast<char*>(&Material.Ambient), sizeof(Material.Ambient));
        File.read(reinterpret_cast<char*>(&Material.Emissive), sizeof(Material.Emissive));
        File.read(reinterpret_cast<char*>(&Material.SpecularScalar), sizeof(Material.SpecularScalar));
        File.read(reinterpret_cast<char*>(&Material.DensityScalar), sizeof(Material.DensityScalar));
        File.read(reinterpret_cast<char*>(&Material.TransparencyScalar), sizeof(Material.TransparencyScalar));
        File.read(reinterpret_cast<char*>(&Material.IlluminanceModel), sizeof(Material.IlluminanceModel));
        Serializer::ReadFString(File, Material.DiffuseTextureName);
        Serializer::ReadFWString(File, Material.DiffuseTexturePath);
        Serializer::ReadFString(File, Material.AmbientTextureName);
        Serializer::ReadFWString(File, Material.AmbientTexturePath);
        Serializer::ReadFString(File, Material.SpecularTextureName);
        Serializer::ReadFWString(File, Material.SpecularTexturePath);
        Serializer::ReadFString(File, Material.BumpTextureName);
        Serializer::ReadFWString(File, Material.BumpTexturePath);
        Serializer::ReadFString(File, Material.AlphaTextureName);
        Serializer::ReadFWString(File, Material.AlphaTexturePath);

        if (!Material.DiffuseTexturePath.empty())
        {
            Textures.AddUnique(Material.DiffuseTexturePath);
        }
        if (!Material.AmbientTexturePath.empty())
        {
            Textures.AddUnique(Material.AmbientTexturePath);
        }
        if (!Material.SpecularTexturePath.empty())
        {
            Textures.AddUnique(Material.SpecularTexturePath);
        }
        if (!Material.BumpTexturePath.empty())
        {
            Textures.AddUnique(Material.BumpTexturePath);
        }
        if (!Material.AlphaTexturePath.empty())
        {
            Textures.AddUnique(Material.AlphaTexturePath);
        }
    }

    // Material Subset
    uint32 SubsetCount = 0;
    File.read(reinterpret_cast<char*>(&SubsetCount), sizeof(SubsetCount));
    OutStaticMesh.MaterialSubsets.SetNum(SubsetCount);
    for (FMaterialSubset& Subset : OutStaticMesh.MaterialSubsets)
    {
        Serializer::ReadFString(File, Subset.MaterialName);
        File.read(reinterpret_cast<char*>(&Subset.IndexStart), sizeof(Subset.IndexStart));
        File.read(reinterpret_cast<char*>(&Subset.IndexCount), sizeof(Subset.IndexCount));
        File.read(reinterpret_cast<char*>(&Subset.MaterialIndex), sizeof(Subset.MaterialIndex));
    }

    // Bounding Box
    File.read(reinterpret_cast<char*>(&OutStaticMesh.BoundingBoxMin), sizeof(FVector));
    File.read(reinterpret_cast<char*>(&OutStaticMesh.BoundingBoxMax), sizeof(FVector));
    
    File.close();

    // Texture Load
    if (Textures.Num() > 0)
    {
        for (const FWString& Texture : Textures)
        {
            if (FEngineLoop::resourceMgr.GetTexture(Texture) == nullptr)
            {
                FEngineLoop::resourceMgr.LoadTextureFromFile(FEngineLoop::graphicDevice.Device, FEngineLoop::graphicDevice.DeviceContext, Texture.c_str());
            }
        }
    }
    
    return true;
}


UMaterial* FManagerOBJ::CreateMaterial(FObjMaterialInfo materialInfo)
{
    if (materialMap[materialInfo.MTLName] != nullptr)
        return materialMap[materialInfo.MTLName];

    UMaterial* newMaterial = FObjectFactory::ConstructObject<UMaterial>();
    newMaterial->SetMaterialInfo(materialInfo);
    materialMap.Add(materialInfo.MTLName, newMaterial);
    return newMaterial;
}

UStaticMesh* FManagerOBJ::CreateStaticMesh(const FString& filePath)
{
    OBJ::FStaticMeshRenderData* staticMeshRenderData = FManagerOBJ::LoadObjStaticMeshAsset(filePath);

    if (staticMeshRenderData == nullptr) return nullptr;

    UStaticMesh* staticMesh = GetStaticMesh(staticMeshRenderData->ObjectName);
    if (staticMesh != nullptr) {
        return staticMesh;
    }

    staticMesh = FObjectFactory::ConstructObject<UStaticMesh>();
    staticMesh->SetData(staticMeshRenderData);

    staticMeshMap.Add(staticMeshRenderData->ObjectName, staticMesh);

    return staticMesh;
}
