#pragma once
#include "Define.h"
#include "Container/Map.h"

struct FPerspectiveData
{
    float FOV;
    float FarClip;
    FVector Location;
    float NearClip;
    FVector Rotation;
};

struct FPrimitiveData
{
    FVector Location;
    FString ObjStaticMeshAsset;
    FVector Rotation;
    FVector Scale;
    FString Type;
public:
    FPrimitiveData() : Location(), ObjStaticMeshAsset(""), Rotation(), Scale(), Type("") {}
};

struct FSceneData
{
    int32 Version;
    int32 NextUUID;
    FPerspectiveData PerspectiveCamera;
    TMap<uint32, FPrimitiveData> Primitives;

public:
    FSceneData() : Version(-1), NextUUID(-1), PerspectiveCamera(), Primitives() {}
};

class FSceneMgr
{
public:
    static bool LoadSceneData(const FString& FileName);
    static bool NewSceneData();
    static bool SaveSceneData(const FString& FileName, FSceneData InSceneData);
public:
    static FSceneData GetCurrentSceneData() { return CurrentSceneData; }

private:
    static FSceneData CurrentSceneData;
};

